#include "GlanceWindow.h"

#include <LayerShellQt/Window>

#include <QFrame>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QWindow>

#include "CalendarWidget.h"
#include "LocalTimeWidget.h"
#include "WorldClockWidget.h"

GlanceWindow::GlanceWindow(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::Tool)
{
    setAttribute(Qt::WA_TranslucentBackground);

    auto *panel = new QFrame(this);
    panel->setObjectName(QStringLiteral("panel"));
    panel->setFixedWidth(360);
    panel->setStyleSheet(QStringLiteral("QFrame#panel { background-color: palette(window); border-radius: 14px; }"));

    auto *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(18, 18, 18, 18);
    panelLayout->setSpacing(14);
    panelLayout->addWidget(new LocalTimeWidget(panel));
    panelLayout->addWidget(new WorldClockWidget(panel));
    panelLayout->addWidget(new CalendarWidget(panel));

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
    layerWindow->setMargins(QMargins(0, 96, 0, 0));
    layerWindow->setDesiredSize(size());
}

void GlanceWindow::toggle()
{
    if (isVisible()) {
        hide();
    } else {
        show();
        raise();
        windowHandle()->requestActivate();
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
