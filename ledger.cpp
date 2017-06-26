#include "ledger.h"
#include "commands.h"
#include "mainwindow.h"
#include <QDate>
#include <QRegularExpression>
#include <QCoreApplication>

int Ledger::COL_DATE = 0;
int Ledger::COL_DESCRIP = 1;
int Ledger::COL_PRICE = 2;
int Ledger::COL_CATEG = 3;
int Ledger::NCOL = 4;
QStringList Ledger::categOptions = (QStringList()
         << "Spending" << "Income" << "Debt" << "Loan" << "Off the book");

Ledger::Ledger(QLineEdit *totalBox, QLineEdit *netBox, QLineEdit *debtBox, QLineEdit *loanBox) : QStandardItemModel()
{
    // variable initialization
    total = 0.0;
    debt = 0.0;
    loan = 0.0;
    net = total - debt + loan;
    // setup header
    QStringList header;
    header << "Date" << "Description" << "Amount" << "Category";
    setHorizontalHeaderLabels(header);
    // set display box
    this->totalBox = totalBox;
    this->netBox   = netBox;
    this->debtBox  = debtBox;
    this->loanBox  = loanBox;
    // setup undo framework
    undoStack = new QUndoStack(this);
}

void Ledger::readFile(QTextStream *inputStream)
{
    if(inputStream){
        if( !(validFile = readCsv(inputStream)) ){
            clear();
            setHorizontalHeaderLabels(QStringList("Error"));
            appendRow(static_cast<QStandardItem*>(new LedgerItem::QStandardItem("Invalid File Format")));
        }
        computeTotal();
    } else{
        appendRow(constructEmptyRow());
        validFile = true;
    }
    undoStack->setClean();
    // setup signals
    connect(this,&QAbstractItemModel::dataChanged, this,&Ledger::computeTotal);
    connect(this,&QAbstractItemModel::rowsRemoved, this,&Ledger::computeTotal);
    connect(this,&QAbstractItemModel::rowsInserted,this,&Ledger::computeTotal);
    connect(undoStack,&QUndoStack::cleanChanged, this,&Ledger::sendChangeSignal);
}

bool Ledger::readCsv(QTextStream *stream)
{
    QChar ch;
    QString field;
    QList<QStandardItem*> row;
    while (!stream->atEnd()) {
        *stream >> ch;
        if (ch == ',' || ch == '\n'){
            if(!checkCsvRow(row, field, ch))
                return false;
        } else if (stream->atEnd()) {
            field.append(ch);
            if(!checkCsvRow(row, field))
                return false;
        } else
            field.append(ch);
    }
    appendRow(constructEmptyRow());
    return true;
}

bool Ledger::checkCsvRow(QList<QStandardItem*> &row, QString &field, QChar ch)
{
    if(field.count("\"")%2 == 0) {
        if (field.startsWith( QChar('\"')) && field.endsWith( QChar('\"') ) ) {
             field.remove( QRegularExpression("^\"") );
             field.remove( QRegularExpression("\"$") );
        }
        field.replace("\"\"", "\"");
        LedgerItem *item = new LedgerItem();
        if( row.size()==COL_DATE ){
            QDate fieldDate = QDate::fromString(field,Qt::ISODate);
            item->setData(QVariant(fieldDate),Qt::EditRole);
            item->setData(Qt::AlignCenter,Qt::TextAlignmentRole);
            item->setData(*MainWindow::numFont,Qt::FontRole);
        } else if( row.size()==COL_DESCRIP ) {
            item->setData(QVariant(field),Qt::EditRole);
            item->setData(int(Qt::AlignLeft|Qt::AlignVCenter),Qt::TextAlignmentRole);
        } else if( row.size()==COL_PRICE ) {
            bool numeric;
            double fieldNum = field.toDouble(&numeric);
            if(numeric && fieldNum!=qQNaN())
                item->setData(QVariant(fieldNum),Qt::EditRole);
            item->setData(int(Qt::AlignRight|Qt::AlignVCenter),Qt::TextAlignmentRole);
            item->setData(*MainWindow::numFont,Qt::FontRole);
        } else if( row.size()==COL_CATEG ) {
            bool isInt;
            int fieldInt = field.toInt(&isInt);
            if(isInt)
                item->setData(QVariant(fieldInt),Qt::EditRole);
            item->setData(int(Qt::AlignLeft|Qt::AlignVCenter),Qt::TextAlignmentRole);
        } else
            item->setText(field);   // Not really necessary
        if(row.size()<NCOL)
            row.append(static_cast<QStandardItem*>(item));
        field.clear();
        if (ch != QChar(',')) {
            QCoreApplication::processEvents(QEventLoop::AllEvents,1);
            if(row.size()!=NCOL) return false;
            appendRow(row);
            row.clear();
        }
    } else {
        field.append(ch);
    }
    return true;
}

void Ledger::writeCsv(QTextStream &stream)
{
    int i, j;
    QString field;
    for( i=0; i<rowCount(); i++ ){
        if( i==rowCount()-1 && isEmptyRow(i) )
            break;
        for( j=0; j<NCOL; j++ ){
            if( !item(i,j) || item(i,j)->data(Qt::EditRole).isNull() )
                field = "";
            else if( j==COL_DATE )
                field = item(i,j)->data(Qt::EditRole).toDate().toString(Qt::ISODate);
            else if( j==COL_PRICE )
                field.setNum(item(i,j)->data(Qt::EditRole).toDouble(),'f',2);
            else{
                field = item(i,j)->data(Qt::EditRole).toString();
                if( field.contains( QRegularExpression("[\",\n]") ) ){
                    field.replace("\"", "\"\"");
                    field.prepend(QChar('\"'));
                    field.append(QChar('\"'));
                }
            }
            stream << field;
            if(j==NCOL-1) stream << '\n';
            else stream << ',';
        }
    }
    stream.flush();
    undoStack->setClean();
}

void Ledger::computeTotal(const QModelIndex &index)
{
    if( !validFile || (index.isValid() && (index.column()!=COL_PRICE && index.column()!=COL_CATEG)) ) return;
    double itemPrice;
    int itemCateg;
    total = 0.0;
    debt  = 0.0;
    loan  = 0.0;
    net   = 0.0;
    for( int i=0; i<rowCount(); i++ ){
        itemPrice = item(i,COL_PRICE)->data(Qt::EditRole).toDouble();
        itemCateg = item(i,COL_CATEG)->data(Qt::EditRole).toInt();
        if(!qIsNaN(itemPrice)){
            if(itemCateg==0)
                total -= itemPrice;
            else if(itemCateg==1)
                total += itemPrice;
            else if(itemCateg==2){
                total += itemPrice;
                debt  += itemPrice;
            } else if(itemCateg==3){
                total -= itemPrice;
                loan  += itemPrice;
            }
        }
    }
    net = total - debt + loan;
    totalBox->setText( QLocale().toString(total,'f',2) );
    netBox  ->setText( QLocale().toString(net,  'f',2) );
    debtBox ->setText( QLocale().toString(debt, 'f',2) );
    loanBox ->setText( QLocale().toString(loan, 'f',2) );
}

QList<QStandardItem *> Ledger::constructEmptyRow()
{
    QList<QStandardItem*> row;
    LedgerItem *item;
    for(int i=0; i<NCOL; i++){
        if(i==COL_DATE){
            item = new LedgerItem;
            item->setData(Qt::AlignCenter,Qt::TextAlignmentRole);
            item->setData(*MainWindow::numFont,Qt::FontRole);
        } else if(i==COL_DESCRIP){
            item = new LedgerItem;
            item->setData(int(Qt::AlignLeft|Qt::AlignVCenter),Qt::TextAlignmentRole);
        } else if(i==COL_PRICE){
            item = new LedgerItem;
            item->setData(int(Qt::AlignRight|Qt::AlignVCenter),Qt::TextAlignmentRole);
            item->setData(*MainWindow::numFont,Qt::FontRole);
        } else if(i==COL_CATEG){
            item = new LedgerItem;
            item->setData(int(Qt::AlignLeft|Qt::AlignVCenter),Qt::TextAlignmentRole);
        } else
            item = new LedgerItem;   // not really necessary
        row.append(static_cast<QStandardItem*>(item));
    }
    return row;
}

void Ledger::sort(int column, Qt::SortOrder order)
{
    undoStack->push(new SortByColumn(column,order,this));
}

bool Ledger::isEmptyRow(int row)
{
    return item(row,COL_DATE  ) ->data(Qt::EditRole).isNull() &&
           item(row,COL_DESCRIP)->data(Qt::EditRole).isNull() &&
           item(row,COL_PRICE ) ->data(Qt::EditRole).isNull() &&
           item(row,COL_CATEG ) ->data(Qt::EditRole).isNull();
}
