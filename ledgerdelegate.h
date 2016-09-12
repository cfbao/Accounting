#ifndef LEDGERDELEGATE_H
#define LEDGERDELEGATE_H
#include <QStyledItemDelegate>

class LedgerDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    LedgerDelegate(QObject *parent = Q_NULLPTR);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &,
                          const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual QString displayText(const QVariant &value, const QLocale &locale) const Q_DECL_OVERRIDE;
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
};

#endif // LEDGERDELEGATE_H
