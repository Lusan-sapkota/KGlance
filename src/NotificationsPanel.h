#pragma once

#include <QWidget>

#include "NotificationMonitor.h"

class QLineEdit;
class QListWidget;
class QToolButton;

class NotificationsPanel : public QWidget {
    Q_OBJECT
public:
    explicit NotificationsPanel(QWidget *parent = nullptr);
    ~NotificationsPanel() override;

    void focusSearch();

private:
    void addEntry(const NotificationEntry &entry);
    void applyFilter(const QString &text);
    void updateDndButton();
    void setDoNotDisturb(bool enabled);
    void clearAll();

    NotificationMonitor *m_monitor;
    QToolButton *m_clearButton;
    QToolButton *m_dndButton;
    QLineEdit *m_searchEdit;
    QListWidget *m_list;
    bool m_doNotDisturb = false;
    uint m_inhibitCookie = 0;
};
