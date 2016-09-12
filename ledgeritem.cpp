#include "ledgeritem.h"
#include "ledger.h"

bool LedgerItem::operator<(const QStandardItem &other) const
{
    if( model() && other.model() && model()==other.model() &&
        column() == Ledger::COL_DESCRIP && other.column() == Ledger::COL_DESCRIP )
    {
        QString a, b;
        a = data(model()->sortRole()).toString();
        b = other.data(other.model()->sortRole()).toString();
        return a.localeAwareCompare(b)<0;
    }
    return QStandardItem::operator <(other);
}
