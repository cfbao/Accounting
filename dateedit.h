#ifndef DATEEDIT_H
#define DATEEDIT_H

#include <QDateEdit>

class DateEdit : public QDateEdit
{
    Q_OBJECT
public:
    explicit DateEdit(QWidget *parent = 0);
    virtual void stepBy(int steps) Q_DECL_OVERRIDE;
protected:
    virtual StepEnabled stepEnabled() const Q_DECL_OVERRIDE;
    virtual void focusInEvent(QFocusEvent *event) Q_DECL_OVERRIDE;
signals:

public slots:
};

#endif // DATEEDIT_H
