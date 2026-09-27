#pragma once

#include <QWidget>

#include "NotificationMonitor.h"

class QLineEdit;
class QToolButton;
class QVBoxLayout;
class NotificationItemWidget;

class NotificationsPanel : public QWidget {
    Q_OBJECT
public:
    explicit NotificationsPanel(QWidget *parent = nullptr);
    ~NotificationsPanel() override;

    void focusSearch();

protected:
    void showEvent(QShowEvent *event) override;

private:
    QList<NotificationItemWidget *> items() const;
    void addEntry(const NotificationEntry &entry);
    void applyFilter(const QString &text);
    void updateDndButton();
    void setDoNotDisturb(bool enabled);
    void clearAll();

    NotificationMonitor *m_monitor;
    QToolButton *m_clearButton;
    QToolButton *m_dndButton;
    QLineEdit *m_searchEdit;
    QWidget *m_list;
    QVBoxLayout *m_listLayout;
    bool m_doNotDisturb = false;
    uint m_inhibitCookie = 0;
};
