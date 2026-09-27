#include "NotificationsPanel.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPixmap>
#include <QRegularExpression>
#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>

namespace {
constexpr int kMaxHistory = 200;
constexpr int kBodyMaxLines = 3;

// The spec lets bodies carry a little markup; keep bare <b>/<i>/<u>/<br> and drop every other
// tag (<img>, <a>, styled spans), so a notification can't pull in images or blow up the layout.
QString sanitizeBody(const QString &body)
{
    static const QRegularExpression disallowedTag(QStringLiteral("<(?!/?(?:b|i|u|br)\\s*/?>)[^>]*>"),
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression strayAngle(QStringLiteral("<(?!/?(?:b|i|u|br)\\s*/?>)"),
        QRegularExpression::CaseInsensitiveOption);
    // Plain "\n" line breaks are allowed alongside markup, but rich text would collapse them;
    // a leftover "<" (as in "5 < 7") would otherwise be parsed as the start of a tag.
    return QString(body).remove(disallowedTag).replace(strayAngle, QStringLiteral("&lt;"))
        .replace(QLatin1Char('\n'), QStringLiteral("<br>"));
}

QString relativeTime(const QDateTime &when)
{
    const QDateTime now = QDateTime::currentDateTime();
    const qint64 minutes = when.secsTo(now) / 60;
    if (minutes < 1) {
        return QObject::tr("now");
    }
    if (minutes < 60) {
        return QObject::tr("%1m ago").arg(minutes);
    }
    const qint64 days = when.date().daysTo(now.date());
    if (days == 0) {
        return when.toString(QStringLiteral("h:mm AP"));
    }
    if (days == 1) {
        return QObject::tr("Yesterday");
    }
    return when.toString(when.date().year() == now.date().year() ? QStringLiteral("MMM d") : QStringLiteral("MMM d, yyyy"));
}

QDBusInterface notificationsInterface()
{
    return QDBusInterface(QStringLiteral("org.kde.plasmashell"),
        QStringLiteral("/org/freedesktop/Notifications"),
        QStringLiteral("org.freedesktop.Notifications"));
}
}

class NotificationItemWidget : public QFrame {
    Q_OBJECT
public:
    explicit NotificationItemWidget(const NotificationEntry &entry, QWidget *parent = nullptr)
        : QFrame(parent)
        , entryId(entry.id)
        , m_appName(entry.appName)
        , m_summary(entry.summary)
        , m_body(sanitizeBody(entry.body))
        , m_timestamp(entry.timestamp)
    {
        setObjectName(QStringLiteral("card"));
        setStyleSheet(QStringLiteral("QFrame#card { background-color: palette(alternate-base); border-radius: 8px; }"));

        auto *iconLabel = new QLabel(this);
        if (!entry.icon.isNull()) {
            iconLabel->setPixmap(QPixmap::fromImage(entry.icon).scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            iconLabel->setPixmap(QIcon::fromTheme(entry.appIcon, QIcon::fromTheme(QStringLiteral("notifications"))).pixmap(20, 20));
        }

        auto *appNameLabel = new QLabel(entry.appName, this);
        appNameLabel->setTextFormat(Qt::PlainText);
        appNameLabel->setStyleSheet(QStringLiteral("font-weight: 600;"));

        m_timeLabel = new QLabel(this);
        m_timeLabel->setStyleSheet(QStringLiteral("color: palette(placeholder-text);"));
        m_timeLabel->setToolTip(QLocale().toString(m_timestamp, QLocale::LongFormat));
        refreshTime();

        auto *headerRow = new QHBoxLayout;
        headerRow->addWidget(iconLabel);
        headerRow->addWidget(appNameLabel);
        headerRow->addStretch();
        headerRow->addWidget(m_timeLabel);

        auto *summaryLabel = new QLabel(entry.summary, this);
        summaryLabel->setTextFormat(Qt::PlainText);
        summaryLabel->setStyleSheet(QStringLiteral("font-weight: 600;"));
        summaryLabel->setWordWrap(true);

        auto *bodyLabel = new QLabel(m_body, this);
        bodyLabel->setTextFormat(Qt::RichText);
        bodyLabel->setWordWrap(true);
        bodyLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop); // so the height cap clips the tail, not the head
        bodyLabel->setMaximumHeight(bodyLabel->fontMetrics().lineSpacing() * kBodyMaxLines);
        bodyLabel->setToolTip(m_body);
        bodyLabel->setStyleSheet(QStringLiteral("color: palette(placeholder-text);"));

        auto *layout = new QVBoxLayout(this);
        layout->addLayout(headerRow);
        layout->addWidget(summaryLabel);
        if (!m_body.isEmpty()) {
            layout->addWidget(bodyLabel);
        }
    }

    quint64 entryId;
    uint realId = 0;

    void refreshTime() { m_timeLabel->setText(relativeTime(m_timestamp)); }

    bool matches(const QString &needle) const
    {
        return m_appName.contains(needle, Qt::CaseInsensitive)
            || m_summary.contains(needle, Qt::CaseInsensitive)
            || m_body.contains(needle, Qt::CaseInsensitive);
    }

private:
    QString m_appName;
    QString m_summary;
    QString m_body;
    QDateTime m_timestamp;
    QLabel *m_timeLabel;
};

NotificationsPanel::NotificationsPanel(QWidget *parent)
    : QWidget(parent)
    , m_monitor(new NotificationMonitor(this))
    , m_clearButton(new QToolButton(this))
    , m_dndButton(new QToolButton(this))
    , m_searchEdit(new QLineEdit(this))
    , m_list(new QWidget)
    , m_listLayout(new QVBoxLayout(m_list))
{
    auto *titleLabel = new QLabel(tr("Notifications"), this);
    titleLabel->setStyleSheet(QStringLiteral("font-weight: 600;"));

    m_clearButton->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear-history")));
    m_clearButton->setToolTip(tr("Clear All"));
    connect(m_clearButton, &QToolButton::clicked, this, &NotificationsPanel::clearAll);

    m_dndButton->setCheckable(true);
    m_dndButton->setToolTip(tr("Do Not Disturb"));
    connect(m_dndButton, &QToolButton::toggled, this, &NotificationsPanel::setDoNotDisturb);
    updateDndButton();

    auto *headerRow = new QHBoxLayout;
    headerRow->addWidget(titleLabel);
    headerRow->addStretch();
    headerRow->addWidget(m_clearButton);
    headerRow->addWidget(m_dndButton);

    m_searchEdit->setPlaceholderText(tr("Search notifications..."));
    connect(m_searchEdit, &QLineEdit::textChanged, this, &NotificationsPanel::applyFilter);

    m_listLayout->setContentsMargins(0, 0, 0, 0);
    m_listLayout->addStretch(); // keeps cards packed at the top

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(m_list);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(headerRow);
    layout->addWidget(m_searchEdit);
    layout->addWidget(scrollArea, 1);

    connect(m_monitor, &NotificationMonitor::notificationReceived, this, &NotificationsPanel::addEntry);
    connect(m_monitor, &NotificationMonitor::notificationIdAssigned, this, [this](quint64 entryId, uint realId) {
        for (auto *item : items()) {
            if (item->entryId == entryId) {
                item->realId = realId;
                break;
            }
        }
    });
}

NotificationsPanel::~NotificationsPanel()
{
    if (m_doNotDisturb && m_inhibitCookie != 0) {
        notificationsInterface().call(QStringLiteral("UnInhibit"), m_inhibitCookie);
    }
}

void NotificationsPanel::focusSearch()
{
    m_searchEdit->setFocus(Qt::ActiveWindowFocusReason);
}

void NotificationsPanel::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    for (auto *item : items()) {
        item->refreshTime();
    }
}

QList<NotificationItemWidget *> NotificationsPanel::items() const
{
    return m_list->findChildren<NotificationItemWidget *>(Qt::FindDirectChildrenOnly);
}

void NotificationsPanel::addEntry(const NotificationEntry &entry)
{
    if (m_doNotDisturb) {
        return;
    }

    // Apps updating a notification in place (players, progress, chats) resend it with
    // replaces_id; drop the stale card so each notification shows once.
    if (entry.replacesId != 0) {
        for (auto *old : items()) {
            if (old->realId == entry.replacesId) {
                delete old;
                break;
            }
        }
    }

    auto *item = new NotificationItemWidget(entry, m_list);
    item->setVisible(item->matches(m_searchEdit->text()));
    m_listLayout->insertWidget(0, item);

    // layout count includes the trailing stretch
    while (m_listLayout->count() - 1 > kMaxHistory) {
        delete m_listLayout->itemAt(m_listLayout->count() - 2)->widget();
    }
}

void NotificationsPanel::applyFilter(const QString &text)
{
    for (auto *item : items()) {
        item->setVisible(item->matches(text));
    }
}

void NotificationsPanel::updateDndButton()
{
    m_dndButton->setIcon(QIcon::fromTheme(m_doNotDisturb
        ? QStringLiteral("notifications-disabled")
        : QStringLiteral("notifications")));
}

void NotificationsPanel::setDoNotDisturb(bool enabled)
{
    m_doNotDisturb = enabled;
    updateDndButton();

    auto notifications = notificationsInterface();
    if (enabled) {
        QDBusReply<uint> reply = notifications.call(QStringLiteral("Inhibit"),
            QStringLiteral("kglance"), QStringLiteral("Do Not Disturb enabled from KGlance"), QVariantMap());
        m_inhibitCookie = reply.isValid() ? reply.value() : 0;
    } else if (m_inhibitCookie != 0) {
        notifications.call(QStringLiteral("UnInhibit"), m_inhibitCookie);
        m_inhibitCookie = 0;
    }
}

void NotificationsPanel::clearAll()
{
    auto notifications = notificationsInterface();
    for (auto *item : items()) {
        if (item->realId != 0) {
            notifications.call(QStringLiteral("CloseNotification"), item->realId);
        }
        delete item;
    }
}

#include "NotificationsPanel.moc"
