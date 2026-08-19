#pragma once

#include <QList>
#include <QString>

struct WorldClockEntry {
    QString label;
    QString timezoneId;
};

class WorldClockConfig {
public:
    static QList<WorldClockEntry> load();
    static QString configPath();
};
