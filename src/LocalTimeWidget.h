#pragma once

#include <QWidget>

class QLabel;
class QTimer;

class LocalTimeWidget : public QWidget {
    Q_OBJECT
public:
    explicit LocalTimeWidget(QWidget *parent = nullptr);

private:
    void refresh();

    QLabel *m_timeLabel;
    QLabel *m_zoneLabel;
    QLabel *m_dateLabel;
    QTimer *m_timer;
};
