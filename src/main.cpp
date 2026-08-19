#include <KGlobalAccel>
#include <QAction>
#include <QApplication>

#include "GlanceWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("kglance"));
    app.setOrganizationName(QStringLiteral("kglance"));
    app.setQuitOnLastWindowClosed(false);

    GlanceWindow window;

    QAction toggleAction;
    toggleAction.setObjectName(QStringLiteral("toggle-kglance"));
    toggleAction.setText(QStringLiteral("Toggle KGlance"));
    QObject::connect(&toggleAction, &QAction::triggered, &window, &GlanceWindow::toggle);

    const QList<QKeySequence> defaultShortcut{QKeySequence(Qt::META | Qt::Key_QuoteLeft)};
    KGlobalAccel::self()->setDefaultShortcut(&toggleAction, defaultShortcut);
    KGlobalAccel::self()->setShortcut(&toggleAction, defaultShortcut);

    return app.exec();
}
