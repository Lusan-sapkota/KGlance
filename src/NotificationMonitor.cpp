#include "NotificationMonitor.h"

#include <dbus/dbus.h>

#include <QAtomicInteger>
#include <QDebug>
#include <QHash>
#include <QThread>
#include <QUrl>

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

// Parses the (iiibiiay) image-data/icon_data hint struct: width, height,
// rowstride, has_alpha, bits_per_sample, channels, then raw pixel bytes.
QImage readImageDataHint(DBusMessageIter *variantIter)
{
    if (dbus_message_iter_get_arg_type(variantIter) != DBUS_TYPE_STRUCT) {
        return {};
    }
    DBusMessageIter s;
    dbus_message_iter_recurse(variantIter, &s);

    auto nextInt32 = [&s](dbus_int32_t *out) {
        if (dbus_message_iter_get_arg_type(&s) != DBUS_TYPE_INT32) {
            return false;
        }
        dbus_message_iter_get_basic(&s, out);
        dbus_message_iter_next(&s);
        return true;
    };

    dbus_int32_t width = 0, height = 0, rowstride = 0, bitsPerSample = 0, channels = 0;
    if (!nextInt32(&width) || !nextInt32(&height) || !nextInt32(&rowstride)) {
        return {};
    }

    if (dbus_message_iter_get_arg_type(&s) != DBUS_TYPE_BOOLEAN) {
        return {};
    }
    dbus_bool_t hasAlpha = FALSE;
    dbus_message_iter_get_basic(&s, &hasAlpha);
    dbus_message_iter_next(&s);

    if (!nextInt32(&bitsPerSample) || !nextInt32(&channels)) {
        return {};
    }

    if (dbus_message_iter_get_arg_type(&s) != DBUS_TYPE_ARRAY) {
        return {};
    }
    DBusMessageIter bytes;
    dbus_message_iter_recurse(&s, &bytes);

    const char *data = nullptr;
    int length = 0;
    dbus_message_iter_get_fixed_array(&bytes, &data, &length);

    if (!data || width <= 0 || height <= 0 || bitsPerSample != 8) {
        return {};
    }

    const QImage::Format format = (channels == 4) ? QImage::Format_RGBA8888 : QImage::Format_RGB888;
    return QImage(reinterpret_cast<const uchar *>(data), width, height, rowstride, format).copy();
}

QString readStringHint(DBusMessageIter *variantIter)
{
    if (dbus_message_iter_get_arg_type(variantIter) != DBUS_TYPE_STRING) {
        return {};
    }
    const char *value = "";
    dbus_message_iter_get_basic(variantIter, &value);
    return QString::fromUtf8(value);
}

// Walks the Notify call's hints (a{sv}) looking for an inline icon. Apps that
// can't express their icon as a static theme name (browsers using favicons,
// chat apps using avatars) put it here instead of in the app_icon argument.
QImage readIconFromHints(DBusMessageIter *hintsIter)
{
    if (dbus_message_iter_get_arg_type(hintsIter) != DBUS_TYPE_ARRAY) {
        return {};
    }

    QString imagePath;
    DBusMessageIter entries;
    dbus_message_iter_recurse(hintsIter, &entries);
    while (dbus_message_iter_get_arg_type(&entries) == DBUS_TYPE_DICT_ENTRY) {
        DBusMessageIter entry;
        dbus_message_iter_recurse(&entries, &entry);

        const QString key = readString(&entry);
        dbus_message_iter_next(&entry);

        if (dbus_message_iter_get_arg_type(&entry) == DBUS_TYPE_VARIANT) {
            DBusMessageIter variant;
            dbus_message_iter_recurse(&entry, &variant);

            if (key == QLatin1String("image-data") || key == QLatin1String("image_data") || key == QLatin1String("icon_data")) {
                const QImage image = readImageDataHint(&variant);
                if (!image.isNull()) {
                    return image;
                }
            } else if (imagePath.isEmpty() && (key == QLatin1String("image-path") || key == QLatin1String("image_path"))) {
                imagePath = readStringHint(&variant);
            }
        }

        dbus_message_iter_next(&entries);
    }

    if (!imagePath.isEmpty() && imagePath.contains(QLatin1Char('/'))) {
        const QString localPath = imagePath.startsWith(QLatin1String("file://"))
            ? QUrl(imagePath).toLocalFile()
            : imagePath;
        return QImage(localPath);
    }
    return {};
}

// One-time resolution of org.kde.plasmashell's unique connection name, so the
// method_return eavesdrop match can be scoped to just its replies instead of
// the whole session bus.
QString resolveUniqueName(DBusConnection *connection, const char *wellKnownName)
{
    DBusMessage *request = dbus_message_new_method_call(
        "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "GetNameOwner");
    dbus_message_append_args(request, DBUS_TYPE_STRING, &wellKnownName, DBUS_TYPE_INVALID);

    DBusError error;
    dbus_error_init(&error);
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(connection, request, -1, &error);
    dbus_message_unref(request);

    QString result;
    if (reply) {
        const char *owner = "";
        if (dbus_message_get_args(reply, nullptr, DBUS_TYPE_STRING, &owner, DBUS_TYPE_INVALID)) {
            result = QString::fromUtf8(owner);
        }
        dbus_message_unref(reply);
    } else {
        dbus_error_free(&error);
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
    void raw(quint64 id, const QString &appName, const QString &appIcon, const QImage &icon,
        const QString &summary, const QString &body, const QStringList &actions);
    void idAssigned(quint64 entryId, uint realId);

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

        const QString notifierUniqueName = resolveUniqueName(connection, "org.kde.plasmashell");
        if (!notifierUniqueName.isEmpty()) {
            const QByteArray rule = QByteArrayLiteral("eavesdrop='true',type='method_return',sender='")
                + notifierUniqueName.toUtf8() + QByteArrayLiteral("'");
            dbus_bus_add_match(connection, rule.constData(), &error);
            if (dbus_error_is_set(&error)) {
                qWarning() << "KGlance: failed to add reply eavesdrop match:" << error.message;
                dbus_error_free(&error);
            }
        } else {
            qWarning() << "KGlance: could not resolve plasmashell's bus name, real notification IDs won't be captured";
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
    quint64 registerPendingNotify(dbus_uint32_t serial)
    {
        const quint64 id = m_nextId++;
        m_pendingSerials.insert(serial, id);
        return id;
    }

    bool resolvePendingReply(dbus_uint32_t replySerial, quint64 *outEntryId)
    {
        const auto it = m_pendingSerials.constFind(replySerial);
        if (it == m_pendingSerials.constEnd()) {
            return false;
        }
        *outEntryId = it.value();
        m_pendingSerials.erase(it);
        return true;
    }

    static DBusHandlerResult filter(DBusConnection *connection, DBusMessage *message, void *userData)
    {
        Q_UNUSED(connection)
        auto *worker = static_cast<Worker *>(userData);

        if (dbus_message_is_method_call(message, "org.freedesktop.Notifications", "Notify")) {
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
            dbus_message_iter_next(&iter);
            const QImage icon = readIconFromHints(&iter);

            const quint64 id = worker->registerPendingNotify(dbus_message_get_serial(message));
            Q_EMIT worker->raw(id, appName, appIcon, icon, summary, body, actions);
            return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
        }

        if (dbus_message_get_type(message) == DBUS_MESSAGE_TYPE_METHOD_RETURN) {
            quint64 entryId = 0;
            if (worker->resolvePendingReply(dbus_message_get_reply_serial(message), &entryId)) {
                DBusMessageIter iter;
                if (dbus_message_iter_init(message, &iter) && dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_UINT32) {
                    dbus_uint32_t realId = 0;
                    dbus_message_iter_get_basic(&iter, &realId);
                    Q_EMIT worker->idAssigned(entryId, realId);
                }
            }
        }

        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED; // observe only, never claim the message
    }

    QAtomicInteger<int> m_stop{0};
    QHash<dbus_uint32_t, quint64> m_pendingSerials;
    quint64 m_nextId = 1;
};

NotificationMonitor::NotificationMonitor(QObject *parent)
    : QObject(parent)
    , m_worker(new Worker(this))
{
    qRegisterMetaType<NotificationEntry>("NotificationEntry");
    connect(m_worker, &Worker::raw, this, &NotificationMonitor::handleRaw);
    connect(m_worker, &Worker::idAssigned, this, &NotificationMonitor::notificationIdAssigned);
    m_worker->start(QThread::LowestPriority);
}

NotificationMonitor::~NotificationMonitor()
{
    m_worker->requestStop();
    m_worker->wait();
}

void NotificationMonitor::handleRaw(quint64 id, const QString &appName, const QString &appIcon, const QImage &icon,
    const QString &summary, const QString &body, const QStringList &actions)
{
    NotificationEntry entry;
    entry.id = id;
    entry.appName = appName;
    entry.appIcon = appIcon;
    entry.icon = icon;
    entry.summary = summary;
    entry.body = body;
    entry.actions = actions;
    entry.timestamp = QDateTime::currentDateTime();

    Q_EMIT notificationReceived(entry);
}

#include "NotificationMonitor.moc"
