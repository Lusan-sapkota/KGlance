#include "LocalTimeWidget.h"

#include <QDateTime>
#include <QLabel>
#include <QTimeZone>
#include <QTimer>
#include <QVBoxLayout>

LocalTimeWidget::LocalTimeWidget(QWidget *parent)
    : QWidget(parent)
    , m_timeLabel(new QLabel(this))
    , m_zoneLabel(new QLabel(this))
    , m_dateLabel(new QLabel(this))
    , m_timer(new QTimer(this))
{
    m_timeLabel->setStyleSheet(QStringLiteral("font-size: 32px; font-weight: 600;"));
    m_zoneLabel->setStyleSheet(QStringLiteral("color: palette(placeholder-text);"));
    m_dateLabel->setStyleSheet(QStringLiteral("color: palette(placeholder-text);"));

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_timeLabel);
    layout->addWidget(m_zoneLabel);
    layout->addWidget(m_dateLabel);

    connect(m_timer, &QTimer::timeout, this, &LocalTimeWidget::refresh);
    refresh();
}

void LocalTimeWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    refresh();
    m_timer->start(1000);
}

void LocalTimeWidget::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    m_timer->stop();
}

void LocalTimeWidget::refresh()
{
    const QDateTime now = QDateTime::currentDateTime();
    const QTimeZone tz = QTimeZone::systemTimeZone();

    m_timeLabel->setText(now.toString(QStringLiteral("h:mm AP")));
    m_zoneLabel->setText(QStringLiteral("%1 (%2)").arg(
        QString::fromUtf8(tz.id()).replace(QLatin1Char('_'), QLatin1Char(' ')),
        tz.abbreviation(now)));
    m_dateLabel->setText(now.toString(QStringLiteral("dddd, MMMM d, yyyy")));
}
