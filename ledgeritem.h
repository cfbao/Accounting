#ifndef LEDGERITEM_H
#define LEDGERITEM_H

#include <QStandardItem>

class LedgerItem : public QStandardItem
{
public:
    LedgerItem() : QStandardItem() {}
    virtual bool operator<(const QStandardItem &other) const Q_DECL_OVERRIDE;
};

#endif // LEDGERITEM_H
