#pragma once

#include <QDateTime>
#include <QObject>

struct NotificationEntry {
    quint64 id = 0;
    QString appName;
    QString appIcon;
    QString summary;
    QString body;
    QStringList actions;
    QDateTime timestamp;
};
Q_DECLARE_METATYPE(NotificationEntry)

class NotificationMonitor : public QObject {
    Q_OBJECT
public:
    explicit NotificationMonitor(QObject *parent = nullptr);
    ~NotificationMonitor() override;

Q_SIGNALS:
    void notificationReceived(const NotificationEntry &entry);

private Q_SLOTS:
    void handleRaw(const QString &appName, const QString &appIcon, const QString &summary,
        const QString &body, const QStringList &actions);

private:
    class Worker;
    Worker *m_worker;
    quint64 m_nextId = 1;
};
