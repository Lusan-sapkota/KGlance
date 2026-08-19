#include "GlanceWindow.h"

#include <LayerShellQt/Window>

#include <QFrame>
#include <QGuiApplication>
#include <QScreen>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QWindow>

#include "CalendarWidget.h"
#include "LocalTimeWidget.h"
#include "NotificationsPanel.h"
#include "WorldClockWidget.h"

GlanceWindow::GlanceWindow(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::Tool)
{
    setAttribute(Qt::WA_TranslucentBackground);

    auto *panel = new QFrame(this);
    panel->setObjectName(QStringLiteral("panel"));
    panel->setStyleSheet(QStringLiteral("QFrame#panel { background-color: palette(window); border-radius: 14px; }"));

    auto *leftColumn = new QVBoxLayout;
    leftColumn->addWidget(new LocalTimeWidget(panel));
    leftColumn->addWidget(new WorldClockWidget(panel));
    leftColumn->addWidget(new CalendarWidget(panel));

    m_notificationsPanel = new NotificationsPanel(panel);
    m_notificationsPanel->setFixedWidth(380);

    auto *panelLayout = new QHBoxLayout(panel);
    panelLayout->setContentsMargins(18, 18, 18, 18);
    panelLayout->setSpacing(20);
    panelLayout->addLayout(leftColumn);
    panelLayout->addWidget(m_notificationsPanel);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(panel);

    adjustSize();
    setFixedSize(size());

    winId(); // force native window creation so LayerShellQt::Window::get() works
    configureLayerShell();

    connect(windowHandle(), &QWindow::activeChanged, this, [this] {
        if (!windowHandle()->isActive() && isVisible()) {
            hide();
        }
    });
}

void GlanceWindow::configureLayerShell()
{
    auto *layerWindow = LayerShellQt::Window::get(windowHandle());
    layerWindow->setLayer(LayerShellQt::Window::LayerTop);
    layerWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityOnDemand);
    layerWindow->setScope(QStringLiteral("kglance"));
    layerWindow->setWantsToBeOnActiveScreen(true); // follow KWin's active output, e.g. the one under the cursor
    layerWindow->setAnchors(LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorTop));
    layerWindow->setDesiredSize(size());

    // Qt's QCursor::pos() is unreliable on Wayland (privacy-restricted), so we can't pick the
    // active screen ourselves; instead react to KWin resolving it via wantsToBeOnActiveScreen.
    auto recenter = [this, layerWindow] {
        QScreen *screen = layerWindow->screen();
        if (!screen) {
            screen = QGuiApplication::primaryScreen();
        }
        const int topMargin = qMax(0, (screen->geometry().height() - height()) / 2);
        layerWindow->setMargins(QMargins(0, topMargin, 0, 0));
    };

    connect(layerWindow, &LayerShellQt::Window::screenChanged, this, recenter);
    recenter();
}

void GlanceWindow::toggle()
{
    if (isVisible()) {
        hide();
    } else {
        show();
        raise();
        windowHandle()->requestActivate();
        m_notificationsPanel->focusSearch();
    }
}

void GlanceWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hide();
        return;
    }
    QWidget::keyPressEvent(event);
}
