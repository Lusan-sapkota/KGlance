#pragma once

#include <QDateTime>
#include <QImage>
#include <QObject>

struct NotificationEntry {
    quint64 id = 0;
    uint replacesId = 0; // Plasma id of the notification this one updates in place, 0 if new
    QString appName;
    QString appIcon;
    QImage icon; // from the Notify call's image-data/image-path hint, if any; may be null
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
    void notificationIdAssigned(quint64 entryId, uint realId);

private Q_SLOTS:
    void handleRaw(quint64 id, uint replacesId, const QString &appName, const QString &appIcon, const QImage &icon,
        const QString &summary, const QString &body, const QStringList &actions);

private:
    class Worker;
    Worker *m_worker;
};
