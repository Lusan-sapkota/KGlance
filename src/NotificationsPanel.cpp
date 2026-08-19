#include "NotificationsPanel.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QListWidget>
#include <QToolButton>
#include <QVBoxLayout>

namespace {
constexpr int kMaxHistory = 200;
}

class NotificationItemWidget : public QFrame {
    Q_OBJECT
public:
    explicit NotificationItemWidget(const NotificationEntry &entry, QWidget *parent = nullptr)
        : QFrame(parent)
        , m_appName(entry.appName)
        , m_summary(entry.summary)
        , m_body(entry.body)
    {
        auto *iconLabel = new QLabel(this);
        iconLabel->setPixmap(QIcon::fromTheme(entry.appIcon, QIcon::fromTheme(QStringLiteral("applications-other"))).pixmap(20, 20));

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
    , m_list(new QListWidget(this))
{
    auto *titleLabel = new QLabel(tr("Notifications"), this);
    titleLabel->setStyleSheet(QStringLiteral("font-weight: 600;"));

    m_clearButton->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear-history")));
    m_clearButton->setToolTip(tr("Clear All"));
    connect(m_clearButton, &QToolButton::clicked, m_list, &QListWidget::clear);

    m_dndButton->setCheckable(true);
    m_dndButton->setToolTip(tr("Do Not Disturb"));
    connect(m_dndButton, &QToolButton::toggled, this, [this](bool checked) {
        m_doNotDisturb = checked;
        updateDndButton();
    });
    updateDndButton();

    auto *headerRow = new QHBoxLayout;
    headerRow->addWidget(titleLabel);
    headerRow->addStretch();
    headerRow->addWidget(m_clearButton);
    headerRow->addWidget(m_dndButton);

    m_searchEdit->setPlaceholderText(tr("Search notifications..."));
    connect(m_searchEdit, &QLineEdit::textChanged, this, &NotificationsPanel::applyFilter);

    m_list->setFrameShape(QFrame::NoFrame);
    m_list->setSelectionMode(QAbstractItemView::NoSelection);
    m_list->setUniformItemSizes(false);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setResizeMode(QListView::Adjust);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(headerRow);
    layout->addWidget(m_searchEdit);
    layout->addWidget(m_list, 1);

    connect(m_monitor, &NotificationMonitor::notificationReceived, this, &NotificationsPanel::addEntry);
}

void NotificationsPanel::focusSearch()
{
    m_searchEdit->setFocus(Qt::ActiveWindowFocusReason);
}

void NotificationsPanel::addEntry(const NotificationEntry &entry)
{
    if (m_doNotDisturb) {
        return;
    }

    auto *widget = new NotificationItemWidget(entry, m_list);
    widget->setFixedWidth(qMax(m_list->viewport()->width() - 4, 100));
    widget->adjustSize(); // re-run the layout now that width is fixed, so wrapped labels report their real height

    auto *item = new QListWidgetItem();
    item->setSizeHint(QSize(widget->width(), widget->height()));
    m_list->insertItem(0, item);
    m_list->setItemWidget(item, widget);

    while (m_list->count() > kMaxHistory) {
        delete m_list->takeItem(m_list->count() - 1);
    }
}

void NotificationsPanel::applyFilter(const QString &text)
{
    for (int i = 0; i < m_list->count(); ++i) {
        auto *item = m_list->item(i);
        auto *widget = qobject_cast<NotificationItemWidget *>(m_list->itemWidget(item));
        item->setHidden(widget && !widget->matches(text));
    }
}

void NotificationsPanel::updateDndButton()
{
    m_dndButton->setIcon(QIcon::fromTheme(m_doNotDisturb
        ? QStringLiteral("notifications-disabled")
        : QStringLiteral("notifications")));
}

#include "NotificationsPanel.moc"
