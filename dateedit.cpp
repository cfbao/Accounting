#include "dateedit.h"

DateEdit::DateEdit(QWidget *parent) : QDateEdit(parent)
{
    setMinimumDate(QDate(1000,1,1));
}

void DateEdit::stepBy(int steps)
{
    if( currentSection()==QDateTimeEdit::DaySection )
        setDate(date().addDays(steps));
    else if( currentSection()==QDateTimeEdit::MonthSection )
        setDate(date().addMonths(steps));
    else if( currentSection()==QDateTimeEdit::YearSection )
        setDate(date().addYears(steps));
}

QAbstractSpinBox::StepEnabled DateEdit::stepEnabled() const
{
    return QAbstractSpinBox::StepUpEnabled|QAbstractSpinBox::StepDownEnabled;
}

void DateEdit::focusInEvent(QFocusEvent *event)
{
    QDateTimeEdit::focusInEvent(event);
    setSelectedSection(QDateTimeEdit::DaySection);
}
