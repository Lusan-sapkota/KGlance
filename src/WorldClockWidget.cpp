#include "WorldClockWidget.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimeZone>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

WorldClockWidget::WorldClockWidget(QWidget *parent)
    : QWidget(parent)
    , m_entries(WorldClockConfig::load())
    , m_labelLabel(new QLabel(this))
    , m_timeLabel(new QLabel(this))
    , m_dateLabel(new QLabel(this))
    , m_prevButton(new QToolButton(this))
    , m_nextButton(new QToolButton(this))
    , m_timer(new QTimer(this))
{
    m_labelLabel->setStyleSheet(QStringLiteral("font-weight: 600;"));
    m_dateLabel->setStyleSheet(QStringLiteral("color: palette(placeholder-text);"));

    m_prevButton->setArrowType(Qt::LeftArrow);
    m_nextButton->setArrowType(Qt::RightArrow);
    connect(m_prevButton, &QToolButton::clicked, this, [this] { step(-1); });
    connect(m_nextButton, &QToolButton::clicked, this, [this] { step(1); });

    auto *nav = new QHBoxLayout;
    nav->addWidget(m_labelLabel);
    nav->addStretch();
    nav->addWidget(m_prevButton);
    nav->addWidget(m_nextButton);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(nav);
    layout->addWidget(m_timeLabel);
    layout->addWidget(m_dateLabel);

    const bool multiple = m_entries.size() > 1;
    m_prevButton->setVisible(multiple);
    m_nextButton->setVisible(multiple);
    setVisible(!m_entries.isEmpty());

    connect(m_timer, &QTimer::timeout, this, &WorldClockWidget::refresh);
    refresh();
}

void WorldClockWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    refresh();
    m_timer->start(1000);
}

void WorldClockWidget::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    m_timer->stop();
}

void WorldClockWidget::step(int delta)
{
    if (m_entries.isEmpty()) {
        return;
    }
    m_index = (m_index + delta + m_entries.size()) % m_entries.size();
    refresh();
}

void WorldClockWidget::refresh()
{
    if (m_entries.isEmpty()) {
        return;
    }

    const WorldClockEntry &entry = m_entries.at(m_index);
    const QTimeZone tz(entry.timezoneId.toUtf8());
    const QDateTime now = QDateTime::currentDateTimeUtc().toTimeZone(tz);

    const int dayDiff = now.date().dayOfYear() - QDateTime::currentDateTime().date().dayOfYear();
    const QString dayNote = dayDiff > 0 ? QStringLiteral(" (+1 day)") : dayDiff < 0 ? QStringLiteral(" (-1 day)") : QString();

    m_labelLabel->setText(QStringLiteral("%1 - %2").arg(entry.label, tz.abbreviation(now)));
    m_timeLabel->setText(now.toString(QStringLiteral("h:mm AP")) + dayNote);
    m_dateLabel->setText(now.toString(QStringLiteral("dddd, MMMM d, yyyy")));
}
