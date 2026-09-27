#include "NotificationsPanel.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>

namespace {
constexpr int kMaxHistory = 200;

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
        , m_body(entry.body)
    {
        auto *iconLabel = new QLabel(this);
        if (!entry.icon.isNull()) {
            iconLabel->setPixmap(QPixmap::fromImage(entry.icon).scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            iconLabel->setPixmap(QIcon::fromTheme(entry.appIcon, QIcon::fromTheme(QStringLiteral("notifications"))).pixmap(20, 20));
        }

        auto *appNameLabel = new QLabel(entry.appName, this);
        appNameLabel->setStyleSheet(QStringLiteral("font-weight: 600;"));

        auto *timeLabel = new QLabel(entry.timestamp.toString(QStringLiteral("h:mm AP")), this);
        timeLabel->setStyleSheet(QStringLiteral("color: palette(placeholder-text);"));

        auto *headerRow = new QHBoxLayout;
        headerRow->addWidget(iconLabel);
        headerRow->addWidget(appNameLabel);
        headerRow->addStretch();
        headerRow->addWidget(timeLabel);

        auto *summaryLabel = new QLabel(entry.summary, this);
        summaryLabel->setStyleSheet(QStringLiteral("font-weight: 600;"));
        summaryLabel->setWordWrap(true);

        auto *bodyLabel = new QLabel(entry.body, this);
        bodyLabel->setWordWrap(true);
        bodyLabel->setStyleSheet(QStringLiteral("color: palette(placeholder-text);"));

        auto *layout = new QVBoxLayout(this);
        layout->addLayout(headerRow);
        layout->addWidget(summaryLabel);
        if (!entry.body.isEmpty()) {
            layout->addWidget(bodyLabel);
        }
    }

    quint64 entryId;
    uint realId = 0;

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

QList<NotificationItemWidget *> NotificationsPanel::items() const
{
    return m_list->findChildren<NotificationItemWidget *>(Qt::FindDirectChildrenOnly);
}

void NotificationsPanel::addEntry(const NotificationEntry &entry)
{
    if (m_doNotDisturb) {
        return;
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
