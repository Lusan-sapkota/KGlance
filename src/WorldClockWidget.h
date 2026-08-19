#pragma once

#include <QWidget>

#include "WorldClockConfig.h"

class QLabel;
class QTimer;
class QToolButton;

class WorldClockWidget : public QWidget {
    Q_OBJECT
public:
    explicit WorldClockWidget(QWidget *parent = nullptr);

private:
    void refresh();
    void step(int delta);

    QList<WorldClockEntry> m_entries;
    int m_index = 0;

    QLabel *m_labelLabel;
    QLabel *m_timeLabel;
    QLabel *m_dateLabel;
    QToolButton *m_prevButton;
    QToolButton *m_nextButton;
    QTimer *m_timer;
};
