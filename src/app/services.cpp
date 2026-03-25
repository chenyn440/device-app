#include "app/services.h"

namespace deviceapp {

ConnectionService::ConnectionService(IDeviceAdapter *deviceAdapter, SettingsRepository *settingsRepository, QObject *parent)
    : QObject(parent), deviceAdapter_(deviceAdapter), settingsRepository_(settingsRepository) {}

DeviceConnectionConfig ConnectionService::recentConnection() const {
    return settingsRepository_->loadRecentConnection();
}

void ConnectionService::connectToDevice(const DeviceConnectionConfig &config) {
    settingsRepository_->saveRecentConnection(config);
    emit recentConnectionChanged(config);
    deviceAdapter_->connectToDevice(config);
}

void ConnectionService::disconnectFromDevice() {
    deviceAdapter_->disconnectFromDevice();
}

ScanControlService::ScanControlService(IDeviceAdapter *deviceAdapter, QObject *parent)
    : QObject(parent), deviceAdapter_(deviceAdapter) {}

void ScanControlService::updateInstrumentStatus(const InstrumentStatus &status) {
    currentStatus_ = status;
}

bool ScanControlService::canStartScan(const ScanSettings &settings, QString *errorMessage) const {
    if (!currentStatus_.connected) {
        if (errorMessage) {
            *errorMessage = "设备未连接";
        }
        return false;
    }
    if (!currentStatus_.vacuum.isReady()) {
        if (errorMessage) {
            *errorMessage = "当前真空度不满足扫描前置条件";
        }
        return false;
    }
    return settings.isValid(errorMessage);
}

void ScanControlService::startScan(const ScanSettings &settings) {
    QString errorMessage;
    if (!canStartScan(settings, &errorMessage)) {
        emit validationFailed(errorMessage);
        return;
    }
    deviceAdapter_->applyScanSettings(settings);
    deviceAdapter_->startScan(settings.mode);
}

void ScanControlService::stopScan() {
    deviceAdapter_->stopScan();
}

void ScanControlService::calibrateMassAxis() {
    deviceAdapter_->calibrateMassAxis();
}

TuneService::TuneService(IDeviceAdapter *deviceAdapter, SettingsRepository *settingsRepository, QObject *parent)
    : QObject(parent),
      deviceAdapter_(deviceAdapter),
      settingsRepository_(settingsRepository),
      currentParameters_(settingsRepository->loadTuneParameters()) {}

TuneParameters TuneService::currentParameters() const {
    return currentParameters_;
}

void TuneService::applyTuneParameters(const TuneParameters &parameters) {
    currentParameters_ = parameters;
    settingsRepository_->saveTuneParameters(parameters);
    deviceAdapter_->applyTuneParameters(parameters);
}

MonitorService::MonitorService(IDeviceAdapter *deviceAdapter, MethodRepository *methodRepository, QObject *parent)
    : QObject(parent), deviceAdapter_(deviceAdapter), methodRepository_(methodRepository) {}

QString MonitorService::saveMethod(const MonitorMethod &method) {
    return methodRepository_->saveMonitorMethod(method);
}

MonitorMethod MonitorService::loadMethod(const QString &path) const {
    return methodRepository_->loadMonitorMethod(path);
}

QStringList MonitorService::listMethods() const {
    return methodRepository_->listMonitorMethods();
}

void MonitorService::applyMethod(const MonitorMethod &method) {
    deviceAdapter_->applyScanSettings(method.scanSettings);
}

PersistenceService::PersistenceService(FrameRepository *frameRepository, QObject *parent)
    : QObject(parent), frameRepository_(frameRepository) {}

QString PersistenceService::saveFrame(const SpectrumFrame &frame) const {
    return frameRepository_->saveFrame(frame);
}

QString PersistenceService::exportFrameCsv(const SpectrumFrame &frame) const {
    return frameRepository_->exportFrameCsv(frame);
}

SpectrumFrame PersistenceService::loadFrame(const QString &path) const {
    return frameRepository_->loadFrame(path);
}

SettingsService::SettingsService(SettingsRepository *settingsRepository, QObject *parent)
    : QObject(parent), settingsRepository_(settingsRepository) {}

DataProcessingSettings SettingsService::dataProcessingSettings() const {
    return settingsRepository_->loadDataProcessingSettings();
}

void SettingsService::saveDataProcessingSettings(const DataProcessingSettings &settings) {
    settingsRepository_->saveDataProcessingSettings(settings);
}

SystemSettings SettingsService::systemSettings() const {
    return settingsRepository_->loadSystemSettings();
}

void SettingsService::saveSystemSettings(const SystemSettings &settings) {
    settingsRepository_->saveSystemSettings(settings);
}

ChartSettings SettingsService::chartSettings() const {
    return settingsRepository_->loadChartSettings();
}

void SettingsService::saveChartSettings(const ChartSettings &settings) {
    settingsRepository_->saveChartSettings(settings);
}

InstrumentControlSettings SettingsService::instrumentControlSettings() const {
    return settingsRepository_->loadInstrumentControlSettings();
}

void SettingsService::saveInstrumentControlSettings(const InstrumentControlSettings &settings) {
    settingsRepository_->saveInstrumentControlSettings(settings);
}

}  // namespace deviceapp
