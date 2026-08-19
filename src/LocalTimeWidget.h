#pragma once

#include <QWidget>

class QLabel;
class QTimer;

class LocalTimeWidget : public QWidget {
    Q_OBJECT
public:
    explicit LocalTimeWidget(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void refresh();

    QLabel *m_timeLabel;
    QLabel *m_zoneLabel;
    QLabel *m_dateLabel;
    QTimer *m_timer;
};
