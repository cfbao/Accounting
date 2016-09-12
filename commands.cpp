#include "commands.h"

DeleteRow::DeleteRow(int row, Ledger *ledger)
{
    this->deletedRow = row;
    this->ledger = ledger;
}

void DeleteRow::redo()
{
    takenRow = ledger->takeRow(deletedRow);
    if(ledger->rowCount()==0) {
        onlyRow = true;
        ledger->appendRow(ledger->constructEmptyRow());
    } else
        onlyRow = false;
}

void DeleteRow::undo()
{
    if(onlyRow)
        ledger->removeRow(0);
    ledger->insertRow(deletedRow,takenRow);
}


MoveRow::MoveRow(int oldRow, int newRow, Ledger *ledger)
{
    this->oldRow = oldRow;
    this->newRow = newRow;
    this->ledger = ledger;
}

void MoveRow::redo()
{
    QList<QStandardItem*> takenRow = ledger->takeRow(oldRow);
    ledger->insertRow(newRow,takenRow);
}

void MoveRow::undo()
{
    QList<QStandardItem*> takenRow = ledger->takeRow(newRow);
    ledger->insertRow(oldRow,takenRow);
}


AddRow::AddRow(int row, Ledger *ledger)
{
    this->row    = row;
    this->ledger = ledger;
}

void AddRow::redo()
{
    ledger->insertRow(row,ledger->constructEmptyRow());
}

void AddRow::undo()
{
    ledger->removeRow(row);
}


ChangeItem::ChangeItem(const QVariant &data, const QModelIndex &index, Ledger *ledger, int role)
{
    this->newData = data;
    this->index   = index;
    this->ledger  = ledger;
    this->role    = role;
    this->oldData = ledger->data(index,role);
    lastRow = (index.row()==ledger->rowCount()-1);
}

void ChangeItem::redo()
{
    ledger->setData(index,newData,role);
    if(lastRow)
        ledger->appendRow(ledger->constructEmptyRow());
}

void ChangeItem::undo()
{
    if(lastRow)
        ledger->removeRow(ledger->rowCount()-1);
    ledger->setData(index,oldData,role);
}

DeleteItems::DeleteItems(const QModelIndexList &indices, Ledger *ledger, int role)
{
    this->indices = indices;
    this->ledger = ledger;
    this->role = role;
    clearedItems.reserve(indices.size());
    foreach(const QModelIndex & index, indices)
        clearedItems.append(ledger->data(index,role));
}

void DeleteItems::redo()
{
    foreach(const QModelIndex & index, indices)
        ledger->setData(index,QVariant(),role);
}

void DeleteItems::undo()
{
    QModelIndexList::const_iterator i;
    QVector<QVariant>::const_iterator j;
    i = indices.cbegin();
    j = clearedItems.cbegin();
    while( i<indices.cend() && j<clearedItems.cend()){
        ledger->setData(*i,*j,role);
        i++;
        j++;
    }
}

SortByColumn::SortByColumn(int column, Qt::SortOrder order, Ledger *ledger)
{
    this->column = column;
    this->order  = order;
    this->ledger = ledger;
    firstTime = true;
    nRow = ledger->rowCount();
}

void SortByColumn::redo()
{
    if(firstTime){
        QVector<QPersistentModelIndex> indices;
        indices.reserve(nRow);
        permut.reserve(nRow);
        for( int i = 0; i < nRow; i++)
            indices.append(QPersistentModelIndex(ledger->index(i,0)));
        ledger->QStandardItemModel::sort(column,order);
        foreach(const QPersistentModelIndex & index, indices)
            permut.append(index.row());
        firstTime = false;
    } else {
        QVector<QList<QStandardItem*> > tmpLedger;
        tmpLedger.resize(nRow);
        for( int i = 0; i < nRow; i++ )
            tmpLedger.replace(permut.at(i),ledger->takeRow(0));
        for( int i = 0; i < nRow; i++ )
            ledger->appendRow(tmpLedger.at(i));
    }
}

void SortByColumn::undo()
{
    QVector<QList<QStandardItem*> > tmpLedger;
    tmpLedger.reserve(nRow);
    for( int i = 0; i < nRow; i++ )
        tmpLedger.append(ledger->takeRow(0));
    for( int i = 0; i < nRow; i++ )
        ledger->appendRow(tmpLedger.at(permut.at(i)));
}
