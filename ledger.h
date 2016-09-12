#ifndef LEDGER_H
#define LEDGER_H

#include <QString>
#include <QTextStream>
#include <QStandardItemModel>
#include <QUndoStack>
#include <QLineEdit>
#include "ledgeritem.h"

class Ledger : public QStandardItemModel
{
    Q_OBJECT
public:
    static int NCOL, COL_DATE, COL_DESCRIP, COL_PRICE, COL_CATEG;
    static QStringList categOptions;
    explicit Ledger(QLineEdit *totalBox, QLineEdit *netBox, QLineEdit *debtBox, QLineEdit *loanBox);
    QUndoStack *undoStack;
    void readFile(QTextStream *inputStream = Q_NULLPTR);
    bool isValidFile() { return validFile; }
    bool readCsv(QTextStream *stream);
    void writeCsv(QTextStream &stream);
    bool isEmptyRow(int row);
    QList<QStandardItem *> constructEmptyRow();
    virtual void sort(int column, Qt::SortOrder order) Q_DECL_OVERRIDE;
signals:
    void ledgerChanged(bool);
private:
    double total, net, debt, loan;
    QLineEdit *totalBox, *netBox, *debtBox, *loanBox;
    bool validFile;
    bool checkCsvRow(QList<QStandardItem*> &row, QString &field, QChar ch = 0);
private slots:
    void computeTotal(const QModelIndex &index=QModelIndex());
    void sendChangeSignal(bool clean) { emit ledgerChanged(!clean); }
};

#endif // LEDGER_H
