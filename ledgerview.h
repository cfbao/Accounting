#ifndef LEDGERVIEW_H
#define LEDGERVIEW_H

#include <QTableView>
#include <QAction>
#include <QUrl>
#include "ledger.h"

class LedgerView : public QTableView
{
    Q_OBJECT
public:
    explicit LedgerView(QWidget *parent = 0);
    void setLedger(Ledger * ledger);
    QList<QAction*> rowActions();
public slots:
    void insertRowAbove();
    void insertRowBelow();
    void moveCurrentRowUp();
    void moveCurrentRowDown();
    void deleteCurrentRow();
    void manageActions();
signals:
    void filesDropped(const QList<QUrl> &urls);
protected:
    virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    virtual void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    virtual void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    virtual void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
protected slots:
    virtual void currentChanged(const QModelIndex &current, const QModelIndex &previous) Q_DECL_OVERRIDE;
private:
    Ledger * ledger;
    QAction* actionInsertRowAbove;
    QAction* actionInsertRowBelow;
    QAction* actionMoveRowUp;
    QAction* actionMoveRowDown;
    QAction* actionDeleteRow;
};

#endif // LEDGERVIEW_H
