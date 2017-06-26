#include "ledgerview.h"
#include "ledgerdelegate.h"
#include "ledgervheader.h"
#include "commands.h"
#include <QKeyEvent>
#include <QKeySequence>
#include <QMimeData>

LedgerView::LedgerView(QWidget *parent) : QTableView(parent)
{
    horizontalHeader()->setSectionsMovable(true);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setVerticalHeader(new LedgerVHeader);
    setSortingEnabled(true);
    setAcceptDrops(true);

    // setItemDelegate removes but does not delete existing delegate
    QAbstractItemDelegate* d = itemDelegate();
    setItemDelegate(new LedgerDelegate);
    delete d;

    actionInsertRowAbove = new QAction(tr("Insert Empty Row &Above"),this);
    actionInsertRowBelow = new QAction(tr("Insert Empty Row &Below"),this);
    actionMoveRowUp      = new QAction(tr("Move Current Row U&p"),   this);
    actionMoveRowDown    = new QAction(tr("Move Current Row &Down"), this);
    actionDeleteRow      = new QAction(tr("De&lete Current Row"),    this);

    actionInsertRowAbove->setShortcut(QKeySequence::fromString("Ctrl+Alt+Up"));
    actionInsertRowBelow->setShortcut(QKeySequence::fromString("Ctrl+Alt+Down"));
    actionMoveRowUp     ->setShortcut(QKeySequence::fromString("Ctrl+Shift+Up"));
    actionMoveRowDown   ->setShortcut(QKeySequence::fromString("Ctrl+Shift+Down"));
    actionDeleteRow     ->setShortcut(QKeySequence::fromString("Ctrl+Delete"));

    actionInsertRowAbove->setShortcutContext(Qt::WindowShortcut);
    actionInsertRowBelow->setShortcutContext(Qt::WindowShortcut);
    actionMoveRowUp     ->setShortcutContext(Qt::WindowShortcut);
    actionMoveRowDown   ->setShortcutContext(Qt::WindowShortcut);
    actionDeleteRow     ->setShortcutContext(Qt::WindowShortcut);

    connect(actionInsertRowAbove,&QAction::triggered, this,&LedgerView::insertRowAbove);
    connect(actionInsertRowBelow,&QAction::triggered, this,&LedgerView::insertRowBelow);
    connect(actionMoveRowUp     ,&QAction::triggered, this,&LedgerView::moveCurrentRowUp);
    connect(actionMoveRowDown   ,&QAction::triggered, this,&LedgerView::moveCurrentRowDown);
    connect(actionDeleteRow     ,&QAction::triggered, this,&LedgerView::deleteCurrentRow);
}

void LedgerView::setLedger(Ledger * ledger)
{
    this->ledger = ledger;

    // setModel removes old model and selection model, but doesn't delete them
    QItemSelectionModel* m = selectionModel();
    setModel(ledger);
    delete m;

    if( ledger && ledger->isValidFile() && ledger->columnCount()==Ledger::NCOL ){
        setColumnWidth(Ledger::COL_DATE,   120);
        setColumnWidth(Ledger::COL_DESCRIP,200);
        setColumnWidth(Ledger::COL_PRICE,  125);
        setColumnWidth(Ledger::COL_CATEG,  110);
        setEditTriggers(DoubleClicked|EditKeyPressed|AnyKeyPressed);
    } else{
        resizeColumnsToContents();
        setEditTriggers(NoEditTriggers);
    }

    manageActions();\
    if(ledger){
        connect(ledger,&QAbstractItemModel::rowsRemoved,  this,&LedgerView::manageActions);
        connect(ledger,&QAbstractItemModel::rowsInserted, this,&LedgerView::manageActions);
        connect(ledger,&QAbstractItemModel::rowsMoved,    this,&LedgerView::manageActions);
        connect(ledger,&QAbstractItemModel::layoutChanged,this,&LedgerView::manageActions);
        connect(ledger,&QObject::destroyed, this,&LedgerView::manageActions);
    }
}

QList<QAction*> LedgerView::rowActions()
{
    QList<QAction*> actions;
    actions << actionInsertRowAbove;
    actions << actionInsertRowBelow;
    actions << actionMoveRowUp;
    actions << actionMoveRowDown;
    actions << actionDeleteRow;
    return actions;
}

void LedgerView::insertRowAbove()
{
    if( !model() || !currentIndex().isValid() ) return;
    int currentRow = currentIndex().row();
    ledger->undoStack->push(new AddRow(currentRow,ledger));
}

void LedgerView::insertRowBelow()
{
    if( !model() || !currentIndex().isValid() ) return;
    int currentRow = currentIndex().row();
    ledger->undoStack->push(new AddRow(currentRow+1,ledger));
}

void LedgerView::moveCurrentRowUp()
{
    if( !model() || !currentIndex().isValid() ) return;
    int currentRow = currentIndex().row();
    int currentCol = currentIndex().column();
    if( currentRow <= 0 ) return;
    ledger->undoStack->push(new MoveRow(currentRow,currentRow-1,ledger));
    QModelIndex index = model()->index(currentRow-1,currentCol);
    setCurrentIndex(index);
    selectionModel()->select(index,QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);
}

void LedgerView::moveCurrentRowDown()
{
    if( !model() || !currentIndex().isValid() ) return;
    int currentRow = currentIndex().row();
    int currentCol = currentIndex().column();
    if( currentRow >= model()->rowCount()-1 ) return;
    ledger->undoStack->push(new MoveRow(currentRow,currentRow+1,ledger));
    QModelIndex index = model()->index(currentRow+1,currentCol);
    setCurrentIndex(index);
    selectionModel()->select(index,QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);
}

void LedgerView::deleteCurrentRow()
{
    if( !model() || !currentIndex().isValid() ) return;
    int currentRow = currentIndex().row();
    int currentCol = currentIndex().column();
    QAbstractItemModel* mod = model();
    if( mod->rowCount()==1 && ledger->isEmptyRow(currentRow) ) return;
    ledger->undoStack->push(new DeleteRow(currentRow,ledger));
    QModelIndex index;
    if( currentRow < mod->rowCount() )
        index = mod->index(currentRow,currentCol);
    else if( currentRow > 0 )
        index = mod->index(currentRow-1,currentCol);
    selectionModel()->setCurrentIndex(index,QItemSelectionModel::Current);
}

void LedgerView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QAbstractItemView::currentChanged(current,previous);
    manageActions();
}

void LedgerView::manageActions()
{
    if(!currentIndex().isValid() || !model()){
        actionInsertRowAbove->setEnabled(false);
        actionInsertRowBelow->setEnabled(false);
        actionMoveRowUp     ->setEnabled(false);
        actionMoveRowDown   ->setEnabled(false);
        actionDeleteRow     ->setEnabled(false);
        return;
    }
    actionInsertRowAbove->setEnabled(true);
    actionInsertRowBelow->setEnabled(true);

    if(currentIndex().row()==0)
        actionMoveRowUp->setEnabled(false);
    else
        actionMoveRowUp->setEnabled(true);
    if(currentIndex().row()==model()->rowCount()-1)
        actionMoveRowDown->setEnabled(false);
    else
        actionMoveRowDown->setEnabled(true);

    if( model()->rowCount()==1 && ledger->isEmptyRow(currentIndex().row()) )
        actionDeleteRow->setEnabled(false);
    else
        actionDeleteRow->setEnabled(true);
}

void LedgerView::keyPressEvent(QKeyEvent *event)
{
    if( event->key()==Qt::Key_Delete && (event->modifiers()&(~Qt::KeypadModifier))==0 ) {
        QModelIndexList selected = selectedIndexes();
        bool empty = true;
        foreach (const QModelIndex & index, selected)
            if(!model()->data(index,Qt::EditRole).isNull())
                empty = false;
        if(!empty)
            ledger->undoStack->push(new DeleteItems(selected,ledger));
    }
    QAbstractItemView::keyPressEvent(event);
}

void LedgerView::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasUrls() && event->proposedAction()==Qt::CopyAction)
        event->accept();
    else
        QTableView::dragEnterEvent(event);
}

void LedgerView::dragMoveEvent(QDragMoveEvent *event)
{
    if(event->mimeData()->hasUrls() && event->proposedAction()==Qt::CopyAction)
        event->acceptProposedAction();
    else
        QTableView::dragMoveEvent(event);
}

void LedgerView::dropEvent(QDropEvent *event)
{
    if(event->mimeData()->hasUrls() && event->proposedAction()==Qt::CopyAction){
        QList<QUrl> urls = event->mimeData()->urls();
        emit filesDropped(urls);
    } else
        QTableView::dropEvent(event);
}
