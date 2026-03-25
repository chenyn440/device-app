#include "core/app_settings.h"

#include <QStandardPaths>

namespace deviceapp {

QString AppSettings::appDataRoot() {
    const QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return path.isEmpty() ? QDir::homePath() + "/.device-app" : path;
}

QString AppSettings::configPath() {
    return appDataRoot() + "/config/settings.json";
}

QString AppSettings::frameDirectory() {
    return appDataRoot() + "/frames";
}

QString AppSettings::monitorMethodDirectory() {
    return appDataRoot() + "/methods/monitor";
}

QString AppSettings::tuneDirectory() {
    return appDataRoot() + "/methods/tune";
}

void AppSettings::ensureDirectories() {
    QDir().mkpath(appDataRoot() + "/config");
    QDir().mkpath(frameDirectory());
    QDir().mkpath(monitorMethodDirectory());
    QDir().mkpath(tuneDirectory());
}

}  // namespace deviceapp
