#pragma once

#include <QObject>

#include "core/types.h"

namespace deviceapp {

class IDeviceAdapter : public QObject {
    Q_OBJECT

public:
    explicit IDeviceAdapter(QObject *parent = nullptr) : QObject(parent) {}
    ~IDeviceAdapter() override = default;

    virtual void connectToDevice(const DeviceConnectionConfig &config) = 0;
    virtual void disconnectFromDevice() = 0;
    virtual void readStatusSnapshot() = 0;
    virtual void applyScanSettings(const ScanSettings &settings) = 0;
    virtual void applyTuneParameters(const TuneParameters &parameters) = 0;
    virtual void applyDataProcessingSettings(const DataProcessingSettings &settings) = 0;
    virtual void setSwitchState(InstrumentSwitch instrumentSwitch, bool on) = 0;
    virtual void startScan(ScanMode mode) = 0;
    virtual void stopScan() = 0;
    virtual void calibrateMassAxis() = 0;
    virtual void saveCurrentFrame() = 0;

signals:
    void connectionChanged(bool connected);
    void statusUpdated(const deviceapp::InstrumentStatus &status);
    void frameUpdated(const deviceapp::SpectrumFrame &frame);
    void calibrationFinished(bool success, const QString &message);
    void frameReadyToPersist(const deviceapp::SpectrumFrame &frame);
    void errorOccurred(const QString &message);
};

}  // namespace deviceapp
