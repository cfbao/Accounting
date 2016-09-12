#include "ledgervheader.h"
#include "ledgerview.h"
#include "commands.h"
#include <QStandardItemModel>
#include <QContextMenuEvent>
#include <QMenu>
#include <QCursor>

LedgerVHeader::LedgerVHeader() : QHeaderView(Qt::Vertical)
{
    setSectionsClickable(true);
    moveReady = false;
    mouseLeftPressIdx = -1;
    mouseRightPressIdx = -1;
    mouseRightReleaseIdx = -1;
    mouseLeftReleaseIdx = -1;
    movingRow = Q_NULLPTR;
}

LedgerVHeader::~LedgerVHeader()
{
    if(movingRow) delete movingRow;
}

void LedgerVHeader::setModel(QAbstractItemModel *model)
{
    QHeaderView::setModel(model);
    ledger = static_cast<Ledger*>(model);
}

void LedgerVHeader::contextMenuEvent(QContextMenuEvent *event)
{
    if(event->reason()==QContextMenuEvent::Mouse){
        mouseRightReleaseIdx = logicalIndexAt(event->pos());
        if( mouseRightReleaseIdx<0 || mouseRightReleaseIdx>=model()->rowCount()) return;
        QMenu menu(this);
        LedgerView* view = static_cast<LedgerView*>(parent());
        menu.addAction("Insert Row &Above",view, &LedgerView::insertRowAbove);
        menu.addAction("Insert Row &Below",view, &LedgerView::insertRowBelow);
        if(mouseRightReleaseIdx>0)
            menu.addAction("Move Row U&p",  view, &LedgerView::moveCurrentRowUp);
        if(mouseRightReleaseIdx<model()->rowCount()-1)
            menu.addAction("Move Row D&own",view, &LedgerView::moveCurrentRowDown);
        if( model()->rowCount()>1 || !ledger->isEmptyRow(mouseRightReleaseIdx) )
            menu.addAction(   "De&lete Row",   view, &LedgerView::deleteCurrentRow);
        menu.exec(event->globalPos());
    }
    QHeaderView::contextMenuEvent(event);
}

void LedgerVHeader::mousePressEvent(QMouseEvent *e)
{
    if(e->button()==Qt::LeftButton){
        prepareMoveRow(e);
    } else if(e->button()==Qt::RightButton)
        mouseRightPressIdx = logicalIndexAt(e->pos());
    QHeaderView::mousePressEvent(e);
}

void LedgerVHeader::mouseDoubleClickEvent(QMouseEvent *e)
{
    QHeaderView::mouseDoubleClickEvent(e);
    prepareMoveRow(e);
}

void LedgerVHeader::mouseMoveEvent(QMouseEvent *e)
{
    if(moveReady && (e->buttons() & Qt::LeftButton)==Qt::LeftButton){
        if(movingRow)
            movingRow->move(0,pos().y()+e->pos().y()-movingRow->height()/2);
        //e->ignore();
    }
    else
        QHeaderView::mouseMoveEvent(e);
}

void LedgerVHeader::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->button()==Qt::RightButton) {
        mouseRightReleaseIdx = logicalIndexAt(e->pos());
        LedgerView *view = static_cast<LedgerView*>(parent());
        view->selectRow(mouseRightReleaseIdx);
        view->selectionModel()->setCurrentIndex(model()->index(mouseRightReleaseIdx,0),QItemSelectionModel::Current);
    } else if(e->button()==Qt::LeftButton) {
        if(movingRow){
            movingRow->hide();
            unsetCursor();
        }
        QPoint mouseLeftReleasePos = e->pos();
        mouseLeftReleaseIdx = logicalIndexAt(mouseLeftReleasePos);
        int newRow = mouseLeftReleaseIdx;
        int oldRow = mouseLeftPressIdx;
        if(moveReady &&
           newRow != oldRow &&
           newRow >= 0 && oldRow < model()->rowCount() &&
           newRow >= 0 && oldRow < model()->rowCount() ){
            int top = sectionViewportPosition(newRow);
            int height = sectionSize(newRow);
            bool upper = (mouseLeftReleasePos.y()-top) < height/2 ;
            LedgerView *view = static_cast<LedgerView*>(parent());
            if(newRow<oldRow && !upper) newRow++;
            if(newRow>oldRow &&  upper) newRow--;
            ledger->undoStack->push(new MoveRow(oldRow,newRow,ledger));
            view->selectRow(newRow);
        }
    }
    QHeaderView::mouseReleaseEvent(e);
}

void LedgerVHeader::prepareMoveRow(QMouseEvent *e)
{
    mouseLeftPressIdx = logicalIndexAt(e->pos());
    LedgerView *view = static_cast<LedgerView*>(parent());
    QModelIndexList selectedIdxes = view->selectionModel()->selectedRows();
    moveReady = (selectedIdxes.size()==1 && selectedIdxes.at(0).row()==mouseLeftPressIdx);
    if(moveReady){
        int height = sectionSize(mouseLeftPressIdx);
        if(!movingRow)
            movingRow = new QRubberBand(QRubberBand::Rectangle,view);
        movingRow->setGeometry(0,pos().y()+e->pos().y()-height/2,view->width(),height);
        movingRow->show();
        setCursor(QCursor(Qt::ClosedHandCursor));
    }
}
