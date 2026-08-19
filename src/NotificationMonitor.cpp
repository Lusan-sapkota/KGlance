#include "NotificationMonitor.h"

#include <dbus/dbus.h>

#include <QAtomicInteger>
#include <QDebug>
#include <QThread>

namespace {

QString readString(DBusMessageIter *iter)
{
    if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_STRING) {
        return {};
    }
    const char *value = "";
    dbus_message_iter_get_basic(iter, &value);
    return QString::fromUtf8(value);
}

QStringList readStringArray(DBusMessageIter *iter)
{
    QStringList result;
    if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY) {
        return result;
    }
    DBusMessageIter items;
    dbus_message_iter_recurse(iter, &items);
    while (dbus_message_iter_get_arg_type(&items) == DBUS_TYPE_STRING) {
        result << readString(&items);
        dbus_message_iter_next(&items);
    }
    return result;
}

}

class NotificationMonitor::Worker : public QThread {
    Q_OBJECT
public:
    using QThread::QThread;

    void requestStop() { m_stop.storeRelaxed(1); }

Q_SIGNALS:
    void raw(const QString &appName, const QString &appIcon, const QString &summary,
        const QString &body, const QStringList &actions);

protected:
    void run() override
    {
        DBusError error;
        dbus_error_init(&error);

        DBusConnection *connection = dbus_bus_get_private(DBUS_BUS_SESSION, &error);
        if (!connection) {
            qWarning() << "KGlance: failed to open private D-Bus connection:" << error.message;
            dbus_error_free(&error);
            return;
        }
        dbus_connection_set_exit_on_disconnect(connection, FALSE);

        dbus_bus_add_match(connection,
            "eavesdrop='true',interface='org.freedesktop.Notifications',member='Notify',type='method_call'", &error);
        if (dbus_error_is_set(&error)) {
            qWarning() << "KGlance: failed to add eavesdrop match:" << error.message;
            dbus_error_free(&error);
        }

        dbus_connection_add_filter(connection, &Worker::filter, this, nullptr);

        // Long timeout: this thread is otherwise fully idle (blocked in poll()), so a longer
        // wait means far fewer wakeups. It only bounds how quickly we notice requestStop().
        while (!m_stop.loadRelaxed() && dbus_connection_get_is_connected(connection)) {
            dbus_connection_read_write_dispatch(connection, 3000);
        }
        if (!m_stop.loadRelaxed()) {
            qWarning() << "KGlance: notification eavesdrop connection dropped, monitoring stopped";
        }

        dbus_connection_remove_filter(connection, &Worker::filter, this);
        dbus_connection_close(connection);
        dbus_connection_unref(connection);
    }

private:
    static DBusHandlerResult filter(DBusConnection *connection, DBusMessage *message, void *userData)
    {
        Q_UNUSED(connection)

        if (!dbus_message_is_method_call(message, "org.freedesktop.Notifications", "Notify")) {
            return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
        }

        DBusMessageIter iter;
        if (!dbus_message_iter_init(message, &iter)) {
            return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
        }

        const QString appName = readString(&iter);
        dbus_message_iter_next(&iter); // replaces_id (uint32), unused
        dbus_message_iter_next(&iter);
        const QString appIcon = readString(&iter);
        dbus_message_iter_next(&iter);
        const QString summary = readString(&iter);
        dbus_message_iter_next(&iter);
        const QString body = readString(&iter);
        dbus_message_iter_next(&iter);
        const QStringList actions = readStringArray(&iter);

        auto *worker = static_cast<Worker *>(userData);
        Q_EMIT worker->raw(appName, appIcon, summary, body, actions);

        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED; // observe only, never claim the message
    }

    QAtomicInteger<int> m_stop{0};
};

NotificationMonitor::NotificationMonitor(QObject *parent)
    : QObject(parent)
    , m_worker(new Worker(this))
{
    qRegisterMetaType<NotificationEntry>("NotificationEntry");
    connect(m_worker, &Worker::raw, this, &NotificationMonitor::handleRaw);
    m_worker->start(QThread::LowestPriority);
}

NotificationMonitor::~NotificationMonitor()
{
    m_worker->requestStop();
    m_worker->wait();
}

void NotificationMonitor::handleRaw(const QString &appName, const QString &appIcon, const QString &summary,
    const QString &body, const QStringList &actions)
{
    NotificationEntry entry;
    entry.id = m_nextId++;
    entry.appName = appName;
    entry.appIcon = appIcon;
    entry.summary = summary;
    entry.body = body;
    entry.actions = actions;
    entry.timestamp = QDateTime::currentDateTime();

    Q_EMIT notificationReceived(entry);
}

#include "NotificationMonitor.moc"
