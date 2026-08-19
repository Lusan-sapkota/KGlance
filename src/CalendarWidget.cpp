#include "CalendarWidget.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QToolButton>
#include <QVBoxLayout>

namespace {
constexpr int kRows = 6;
constexpr int kCols = 7;
}

CalendarWidget::CalendarWidget(QWidget *parent)
    : QWidget(parent)
    , m_viewedMonth(QDate::currentDate())
    , m_monthLabel(new QLabel(this))
    , m_grid(new QGridLayout)
{
    auto *prevButton = new QToolButton(this);
    auto *nextButton = new QToolButton(this);
    prevButton->setArrowType(Qt::LeftArrow);
    nextButton->setArrowType(Qt::RightArrow);
    connect(prevButton, &QToolButton::clicked, this, [this] { shiftMonth(-1); });
    connect(nextButton, &QToolButton::clicked, this, [this] { shiftMonth(1); });

    m_monthLabel->setStyleSheet(QStringLiteral("font-weight: 600;"));

    auto *nav = new QHBoxLayout;
    nav->addWidget(m_monthLabel);
    nav->addStretch();
    nav->addWidget(prevButton);
    nav->addWidget(nextButton);

    for (int col = 0; col < kCols; ++col) {
        const QDate weekday(2024, 1, 7 + col); // arbitrary Sun-Sat week, only used for its weekday names
        auto *header = new QLabel(QLocale::system().dayName(weekday.dayOfWeek(), QLocale::ShortFormat), this);
        header->setAlignment(Qt::AlignCenter);
        header->setStyleSheet(QStringLiteral("color: palette(placeholder-text);"));
        m_grid->addWidget(header, 0, col);
    }

    for (int row = 0; row < kRows; ++row) {
        for (int col = 0; col < kCols; ++col) {
            auto *cell = new QLabel(this);
            cell->setAlignment(Qt::AlignCenter);
            cell->setFixedSize(32, 32);
            m_grid->addWidget(cell, row + 1, col);
            m_dayCells.append(cell);
        }
    }

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(nav);
    layout->addLayout(m_grid);

    refresh();
}

void CalendarWidget::shiftMonth(int delta)
{
    m_viewedMonth = m_viewedMonth.addMonths(delta);
    refresh();
}

void CalendarWidget::refresh()
{
    m_monthLabel->setText(QLocale::system().toString(m_viewedMonth, QStringLiteral("MMMM yyyy")));

    const QDate firstOfMonth(m_viewedMonth.year(), m_viewedMonth.month(), 1);
    const int leadingBlanks = firstOfMonth.dayOfWeek() % 7; // Qt: Mon=1..Sun=7, we want Sun-first
    const int daysInMonth = firstOfMonth.daysInMonth();
    const QDate today = QDate::currentDate();

    for (int i = 0; i < m_dayCells.size(); ++i) {
        QLabel *cell = m_dayCells[i];
        const int day = i - leadingBlanks + 1;

        if (day < 1 || day > daysInMonth) {
            cell->clear();
            cell->setStyleSheet(QString());
            continue;
        }

        const QDate cellDate(m_viewedMonth.year(), m_viewedMonth.month(), day);
        cell->setText(QString::number(day));
        cell->setStyleSheet(cellDate == today
            ? QStringLiteral("background: palette(highlight); color: palette(highlighted-text); border-radius: 16px;")
            : QString());
    }
}
