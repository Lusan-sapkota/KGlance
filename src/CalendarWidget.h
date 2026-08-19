#pragma once

#include <QDate>
#include <QWidget>

class QGridLayout;
class QLabel;
class QToolButton;

class CalendarWidget : public QWidget {
    Q_OBJECT
public:
    explicit CalendarWidget(QWidget *parent = nullptr);

private:
    void refresh();
    void shiftMonth(int delta);

    QDate m_viewedMonth;
    QLabel *m_monthLabel;
    QGridLayout *m_grid;
    QList<QLabel *> m_dayCells;
};
