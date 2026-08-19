#include "WorldClockConfig.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

QString WorldClockConfig::configPath()
{
    return QDir::homePath() + QStringLiteral("/.config/kglance/worldclocks.json");
}

QList<WorldClockEntry> WorldClockConfig::load()
{
    QList<WorldClockEntry> entries;

    QFile file(configPath());
    if (!file.open(QIODevice::ReadOnly)) {
        return entries;
    }

    const auto doc = QJsonDocument::fromJson(file.readAll());
    for (const auto &value : doc.array()) {
        const auto obj = value.toObject();
        const QString tz = obj.value(QStringLiteral("timezone")).toString();
        if (tz.isEmpty()) {
            continue;
        }
        entries.append({obj.value(QStringLiteral("label")).toString(tz), tz});
    }

    return entries;
}
