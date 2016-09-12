#include "ledgerdelegate.h"
#include "ledger.h"
#include "commands.h"
#include "dateedit.h"
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCalendarWidget>
#include <QPainter>
#include <QFontMetrics>
#include <QCompleter>

LedgerDelegate::LedgerDelegate(QObject *parent) : QStyledItemDelegate(parent){}

QWidget *LedgerDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.column()==Ledger::COL_DATE){
        DateEdit *dateEditor = new DateEdit(parent);
        dateEditor->setCalendarPopup(true);
        dateEditor->setDisplayFormat("yyyy-MM-dd");
        dateEditor->setFont( index.model()->data(index,Qt::FontRole).value<QFont>() );
        dateEditor->setAlignment(Qt::AlignHCenter);
        dateEditor->calendarWidget()->setGridVisible(true);
        return dateEditor;
    } else if(index.column()==Ledger::COL_DESCRIP) {
        QLineEdit *textEditor = new QLineEdit(parent);
        QStringList descripList;
        int start = std::max(index.row()-100,0);
        int end   = std::min(index.row()+101,index.model()->rowCount());
        for(int i = start; i < end; i++){
            QString itemString = index.model()->
                    data( index.sibling(i,Ledger::COL_DESCRIP),Qt::EditRole ).toString();
            if(!descripList.contains(itemString))
                descripList.append(itemString);
        }
        QCompleter *completer = new QCompleter(descripList,textEditor);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setFilterMode(Qt::MatchContains);
        textEditor->setFont(option.font);
        textEditor->setCompleter(completer);
        return textEditor;
    } else if(index.column()==Ledger::COL_PRICE){
        QDoubleSpinBox *numberEditor = new QDoubleSpinBox(parent);
        numberEditor->setSingleStep(1.0);
        numberEditor->setDecimals(2);
        numberEditor->setMinimum(std::numeric_limits<double>::lowest());
        numberEditor->setMaximum(std::numeric_limits<double>::max());
        numberEditor->setAlignment(Qt::AlignRight);
        numberEditor->setGroupSeparatorShown(true);
        numberEditor->setFont( index.model()->data(index,Qt::FontRole).value<QFont>() );
        return numberEditor;
    } else if(index.column()==Ledger::COL_CATEG){
        QComboBox *categEditor = new QComboBox(parent);
        categEditor->insertItems(0,Ledger::categOptions);
        return categEditor;
    } else
        return QStyledItemDelegate::createEditor(parent,option,index);
}

void LedgerDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if(index.column()==Ledger::COL_DATE){
        DateEdit *dateEditor = static_cast<DateEdit*>(editor);
        QDate date = index.model()->data(index,Qt::EditRole).toDate();
        if(date.isNull())
            dateEditor->setDate(QDate::currentDate());
        else
            dateEditor->setDate( date );
    } else if(index.column()==Ledger::COL_DESCRIP) {
        QLineEdit *textEditor = static_cast<QLineEdit*>(editor);
        textEditor->setText(index.model()->data(index,Qt::EditRole).toString());
    } else if(index.column()==Ledger::COL_PRICE){
        QDoubleSpinBox *numberEditor = static_cast<QDoubleSpinBox*>(editor);
        double value = index.model()->data(index,Qt::EditRole).toDouble();
        if(qIsNaN(value))
            numberEditor->setValue( 0.0 );
        else
            numberEditor->setValue( value );
    } else if(index.column()==Ledger::COL_CATEG){
        QComboBox *categEditor = static_cast<QComboBox*>(editor);
        int categ = index.model()->data(index,Qt::EditRole).toInt();
        categEditor->setCurrentIndex(categ);
    }
}

void LedgerDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    Ledger *ledger = static_cast<Ledger*>(model);
    QVariant newData;
    if(index.column()==Ledger::COL_DATE){
        DateEdit *dateEditor = static_cast<DateEdit*>(editor);
        dateEditor->interpretText();
        newData.setValue(dateEditor->date());
    } else if(index.column()==Ledger::COL_DESCRIP) {
        QLineEdit *textEditor = static_cast<QLineEdit*>(editor);
        QString text = textEditor->text();
        if(!text.isEmpty())
            newData.setValue(text);
    } else if(index.column()==Ledger::COL_PRICE){
        QDoubleSpinBox *numberEditor = static_cast<QDoubleSpinBox*>(editor);
        numberEditor->interpretText();
        newData.setValue(numberEditor->value());
    } else if(index.column()==Ledger::COL_CATEG){
        QComboBox *categEditor = static_cast<QComboBox*>(editor);
        newData.setValue(categEditor->currentIndex());
    }
    if(ledger->data(index,Qt::EditRole)!=newData)
        ledger->undoStack->push(new ChangeItem(newData,index,ledger));
}

QString LedgerDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    if(value.isNull()) return "";
    if(QMetaType::Type(value.type())==QMetaType::Double) {
        double number = value.toDouble();
        if(qIsNaN(number))
            return "";
        else {
            QString numStr = QLocale().toString(number,'f',2);
            numStr += " ";
            return numStr;
        }
    } else if(QMetaType::Type(value.type())==QMetaType::QDate)
        return value.toDate().toString("yyyy-MM-dd");
    else if(QMetaType::Type(value.type())==QMetaType::Int) {
        int categ=value.toInt();
        if(categ >= 0 && categ < Ledger::categOptions.length())
            return Ledger::categOptions.at(categ);
        else
            return "";
    } else
        return QStyledItemDelegate::displayText(value,locale);
}

void LedgerDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter,option,index);
    if(index.column()!=Ledger::COL_PRICE) return;
    painter->save();
    painter->setFont(index.model()->data(index,Qt::FontRole).value<QFont>());
    painter->drawText(option.rect.adjusted(3,0,0,0),Qt::AlignLeft|Qt::AlignVCenter,QLocale().currencySymbol());
    painter->restore();
}

QSize LedgerDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.column()==Ledger::COL_PRICE){
        QFontMetrics metrics(index.model()->data(index,Qt::FontRole).value<QFont>());
        double number = index.model()->data(index,Qt::EditRole).toDouble();
        QString numStr = "$ " + QLocale().toString(number,'f',2) + " ";
        int width  = metrics.width(numStr)+6;
        int height = metrics.height();
        return QSize(width,height);
    }
    else
        return QStyledItemDelegate::sizeHint(option,index);
}

void LedgerDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::updateEditorGeometry(editor,option,index);
}
