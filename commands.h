#ifndef COMMANDS_H
#define COMMANDS_H

#include "ledger.h"
#include <QUndoCommand>
#include <QPersistentModelIndex>

class DeleteRow : public QUndoCommand {
public:
    DeleteRow(int row, Ledger *ledger);
    void redo() Q_DECL_OVERRIDE;
    void undo() Q_DECL_OVERRIDE;
private:
    int deletedRow;
    bool onlyRow;
    Ledger *ledger;
    QList<QStandardItem*> takenRow;
};

class MoveRow : public QUndoCommand {
public:
    MoveRow(int oldRow, int newRow, Ledger *ledger);
    void redo() Q_DECL_OVERRIDE;
    void undo() Q_DECL_OVERRIDE;
private:
    int oldRow, newRow;
    Ledger *ledger;
};


class AddRow : public QUndoCommand {
public:
    AddRow(int row, Ledger *ledger);
    void redo() Q_DECL_OVERRIDE;
    void undo() Q_DECL_OVERRIDE;
private:
    int row;
    Ledger *ledger;
};

class ChangeItem : public QUndoCommand {
public:
    ChangeItem(const QVariant &data, const QModelIndex &index, Ledger *ledger, int role=Qt::EditRole);
    void redo() Q_DECL_OVERRIDE;
    void undo() Q_DECL_OVERRIDE;
private:
    QVariant newData, oldData;
    int role;
    bool lastRow;
    QModelIndex index;
    Ledger *ledger;
};

class DeleteItems : public QUndoCommand {
public:
    DeleteItems(const QModelIndexList &indices, Ledger *ledger, int role=Qt::EditRole);
    void redo() Q_DECL_OVERRIDE;
    void undo() Q_DECL_OVERRIDE;
private:
    QModelIndexList indices;
    QVector<QVariant> clearedItems;
    int role;
    Ledger *ledger;
};

class SortByColumn : public QUndoCommand {
public:
    SortByColumn(int column, Qt::SortOrder order, Ledger *ledger);
    void redo() Q_DECL_OVERRIDE;
    void undo() Q_DECL_OVERRIDE;
private:
    int column;
    Qt::SortOrder order;
    Ledger *ledger;
    bool firstTime;
    int nRow;
    QVector<int> permut;
};

#endif // COMMANDS_H
