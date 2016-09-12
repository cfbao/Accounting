#ifndef LEDGERVHEADER_H
#define LEDGERVHEADER_H

#include <QHeaderView>
#include <QMouseEvent>
#include <QRubberBand>
#include "ledger.h"

class LedgerVHeader : public QHeaderView
{
    Q_OBJECT
public:
    explicit LedgerVHeader();
    ~LedgerVHeader();
    virtual void setModel(QAbstractItemModel *model) Q_DECL_OVERRIDE;
protected:
    virtual void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
    virtual void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    virtual void mouseDoubleClickEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    virtual void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
private:
    int mouseLeftPressIdx, mouseRightPressIdx, mouseRightReleaseIdx, mouseLeftReleaseIdx;
    void prepareMoveRow(QMouseEvent *e);
    bool moveReady;
    QRubberBand *movingRow;
    Ledger * ledger;
};

#endif // LEDGERVHEADER_H
