#pragma once

#include <QObject>

#include "device/device_adapter.h"
#include "storage/repositories.h"

namespace deviceapp {

class ConnectionService : public QObject {
    Q_OBJECT

public:
    ConnectionService(IDeviceAdapter *deviceAdapter, SettingsRepository *settingsRepository, QObject *parent = nullptr);

    DeviceConnectionConfig recentConnection() const;
    void connectToDevice(const DeviceConnectionConfig &config);
    void disconnectFromDevice();

signals:
    void recentConnectionChanged(const deviceapp::DeviceConnectionConfig &config);

private:
    IDeviceAdapter *deviceAdapter_;
    SettingsRepository *settingsRepository_;
};

class ScanControlService : public QObject {
    Q_OBJECT

public:
    explicit ScanControlService(IDeviceAdapter *deviceAdapter, QObject *parent = nullptr);

    void updateInstrumentStatus(const InstrumentStatus &status);
    bool canStartScan(const ScanSettings &settings, QString *errorMessage) const;
    void startScan(const ScanSettings &settings);
    void stopScan();
    void calibrateMassAxis();

signals:
    void validationFailed(const QString &message);

private:
    IDeviceAdapter *deviceAdapter_;
    InstrumentStatus currentStatus_;
};

class TuneService : public QObject {
    Q_OBJECT

public:
    TuneService(IDeviceAdapter *deviceAdapter, SettingsRepository *settingsRepository, QObject *parent = nullptr);

    TuneParameters currentParameters() const;
    void applyTuneParameters(const TuneParameters &parameters);

private:
    IDeviceAdapter *deviceAdapter_;
    SettingsRepository *settingsRepository_;
    TuneParameters currentParameters_;
};

class MonitorService : public QObject {
    Q_OBJECT

public:
    MonitorService(IDeviceAdapter *deviceAdapter, MethodRepository *methodRepository, QObject *parent = nullptr);

    QString saveMethod(const MonitorMethod &method);
    MonitorMethod loadMethod(const QString &path) const;
    QStringList listMethods() const;
    void applyMethod(const MonitorMethod &method);

private:
    IDeviceAdapter *deviceAdapter_;
    MethodRepository *methodRepository_;
};

class PersistenceService : public QObject {
    Q_OBJECT

public:
    PersistenceService(FrameRepository *frameRepository, QObject *parent = nullptr);

    QString saveFrame(const SpectrumFrame &frame) const;
    QString exportFrameCsv(const SpectrumFrame &frame) const;
    SpectrumFrame loadFrame(const QString &path) const;

private:
    FrameRepository *frameRepository_;
};

class SettingsService : public QObject {
    Q_OBJECT

public:
    explicit SettingsService(SettingsRepository *settingsRepository, QObject *parent = nullptr);

    DataProcessingSettings dataProcessingSettings() const;
    void saveDataProcessingSettings(const DataProcessingSettings &settings);
    SystemSettings systemSettings() const;
    void saveSystemSettings(const SystemSettings &settings);
    ChartSettings chartSettings() const;
    void saveChartSettings(const ChartSettings &settings);
    InstrumentControlSettings instrumentControlSettings() const;
    void saveInstrumentControlSettings(const InstrumentControlSettings &settings);

private:
    SettingsRepository *settingsRepository_;
};

}  // namespace deviceapp
