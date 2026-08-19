#include "WorldClockConfig.h"

#include <KConfig>
#include <KConfigGroup>

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>

namespace {

QString labelForTimeZone(const QString &timeZoneId)
{
    const QString cityPart = timeZoneId.section(QLatin1Char('/'), -1);
    return QString(cityPart).replace(QLatin1Char('_'), QLatin1Char(' '));
}

QList<WorldClockEntry> loadFromPlasmaDigitalClock()
{
    QList<WorldClockEntry> entries;
    QSet<QString> seen;

    KConfig config(QStringLiteral("plasma-org.kde.plasma.desktop-appletsrc"), KConfig::SimpleConfig);
    KConfigGroup containments = config.group(QStringLiteral("Containments"));

    for (const QString &containmentId : containments.groupList()) {
        KConfigGroup applets = containments.group(containmentId).group(QStringLiteral("Applets"));
        for (const QString &appletId : applets.groupList()) {
            KConfigGroup applet = applets.group(appletId);
            if (applet.readEntry("plugin") != QLatin1String("org.kde.plasma.digitalclock")) {
                continue;
            }

            KConfigGroup appearance = applet.group(QStringLiteral("Configuration")).group(QStringLiteral("Appearance"));
            const QStringList zones = appearance.readEntry("selectedTimeZones", QStringList());
            for (const QString &zone : zones) {
                if (zone == QLatin1String("Local") || seen.contains(zone)) {
                    continue;
                }
                seen.insert(zone);
                entries.append({labelForTimeZone(zone), zone});
            }
        }
    }

    return entries;
}

}

QString WorldClockConfig::configPath()
{
    return QDir::homePath() + QStringLiteral("/.config/kglance/worldclocks.json");
}

QList<WorldClockEntry> WorldClockConfig::load()
{
    QList<WorldClockEntry> entries;

    QFile file(configPath());
    if (file.open(QIODevice::ReadOnly)) {
        const auto doc = QJsonDocument::fromJson(file.readAll());
        for (const auto &value : doc.array()) {
            const auto obj = value.toObject();
            const QString tz = obj.value(QStringLiteral("timezone")).toString();
            if (tz.isEmpty()) {
                continue;
            }
            entries.append({obj.value(QStringLiteral("label")).toString(tz), tz});
        }
    }

    if (!entries.isEmpty()) {
        return entries;
    }

    // No KGlance-specific override: reuse whatever cities are already configured
    // in Plasma's own Digital Clock widget, so nothing has to be set up twice.
    return loadFromPlasmaDigitalClock();
}
