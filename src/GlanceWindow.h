#pragma once

#include <QWidget>

class GlanceWindow : public QWidget {
    Q_OBJECT
public:
    explicit GlanceWindow(QWidget *parent = nullptr);

public Q_SLOTS:
    void toggle();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void configureLayerShell();
};
