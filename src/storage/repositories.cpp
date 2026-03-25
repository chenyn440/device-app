#include "storage/repositories.h"

#include <QFile>
#include <QJsonDocument>
#include <QTextStream>

#include "core/app_settings.h"

namespace deviceapp {

namespace {

QJsonObject readJsonObject(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    const auto document = QJsonDocument::fromJson(file.readAll());
    return document.object();
}

void writeJsonObject(const QString &path, const QJsonObject &object) {
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
    }
}

QString stampedName(const QString &prefix, const QString &suffix) {
    return prefix + "_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + suffix;
}

}  // namespace

SettingsRepository::SettingsRepository(QObject *parent) : QObject(parent) {
    AppSettings::ensureDirectories();
}

DeviceConnectionConfig SettingsRepository::loadRecentConnection() const {
    return connectionConfigFromJson(readJsonObject(AppSettings::configPath()).value("recentConnection").toObject());
}

void SettingsRepository::saveRecentConnection(const DeviceConnectionConfig &config) const {
    QJsonObject root = readJsonObject(AppSettings::configPath());
    root["recentConnection"] = toJson(config);
    writeJsonObject(AppSettings::configPath(), root);
}

TuneParameters SettingsRepository::loadTuneParameters() const {
    return tuneParametersFromJson(readJsonObject(AppSettings::configPath()).value("tuneParameters").toObject());
}

void SettingsRepository::saveTuneParameters(const TuneParameters &parameters) const {
    QJsonObject root = readJsonObject(AppSettings::configPath());
    root["tuneParameters"] = toJson(parameters);
    writeJsonObject(AppSettings::configPath(), root);
}

DataProcessingSettings SettingsRepository::loadDataProcessingSettings() const {
    return dataProcessingSettingsFromJson(readJsonObject(AppSettings::configPath()).value("dataProcessing").toObject());
}

void SettingsRepository::saveDataProcessingSettings(const DataProcessingSettings &settings) const {
    QJsonObject root = readJsonObject(AppSettings::configPath());
    root["dataProcessing"] = toJson(settings);
    writeJsonObject(AppSettings::configPath(), root);
}

SystemSettings SettingsRepository::loadSystemSettings() const {
    return systemSettingsFromJson(readJsonObject(AppSettings::configPath()).value("systemSettings").toObject());
}

void SettingsRepository::saveSystemSettings(const SystemSettings &settings) const {
    QJsonObject root = readJsonObject(AppSettings::configPath());
    root["systemSettings"] = toJson(settings);
    writeJsonObject(AppSettings::configPath(), root);
}

ChartSettings SettingsRepository::loadChartSettings() const {
    return chartSettingsFromJson(readJsonObject(AppSettings::configPath()).value("chartSettings").toObject());
}

void SettingsRepository::saveChartSettings(const ChartSettings &settings) const {
    QJsonObject root = readJsonObject(AppSettings::configPath());
    root["chartSettings"] = toJson(settings);
    writeJsonObject(AppSettings::configPath(), root);
}

InstrumentControlSettings SettingsRepository::loadInstrumentControlSettings() const {
    return instrumentControlSettingsFromJson(readJsonObject(AppSettings::configPath()).value("instrumentControl").toObject());
}

void SettingsRepository::saveInstrumentControlSettings(const InstrumentControlSettings &settings) const {
    QJsonObject root = readJsonObject(AppSettings::configPath());
    root["instrumentControl"] = toJson(settings);
    writeJsonObject(AppSettings::configPath(), root);
}

MethodRepository::MethodRepository(QObject *parent) : QObject(parent) {
    AppSettings::ensureDirectories();
}

QString MethodRepository::saveMonitorMethod(const MonitorMethod &method) const {
    const QString name = method.name.trimmed().isEmpty() ? "method" : method.name.trimmed();
    const QString path = AppSettings::monitorMethodDirectory() + "/" + name + ".json";
    writeJsonObject(path, toJson(method));
    return path;
}

MonitorMethod MethodRepository::loadMonitorMethod(const QString &filePath) const {
    return monitorMethodFromJson(readJsonObject(filePath));
}

QStringList MethodRepository::listMonitorMethods() const {
    QDir dir(AppSettings::monitorMethodDirectory());
    return dir.entryList(QStringList() << "*.json", QDir::Files, QDir::Name);
}

FrameRepository::FrameRepository(QObject *parent) : QObject(parent) {
    AppSettings::ensureDirectories();
}

QString FrameRepository::saveFrame(const SpectrumFrame &frame) const {
    const QString path = AppSettings::frameDirectory() + "/" + stampedName("frame", ".json");
    writeJsonObject(path, toJson(frame));
    return path;
}

SpectrumFrame FrameRepository::loadFrame(const QString &filePath) const {
    return spectrumFrameFromJson(readJsonObject(filePath));
}

QString FrameRepository::exportFrameCsv(const SpectrumFrame &frame, const QString &baseName) const {
    const QString fileName = baseName.isEmpty() ? stampedName("frame", ".csv") : baseName + ".csv";
    const QString path = AppSettings::frameDirectory() + "/" + fileName;
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << "mass,intensity\n";
        for (int i = 0; i < frame.masses.size() && i < frame.intensities.size(); ++i) {
            stream << frame.masses[i] << "," << frame.intensities[i] << "\n";
        }
    }
    return path;
}

}  // namespace deviceapp
