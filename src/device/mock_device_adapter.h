#pragma once

#include <QTimer>

#include "device/device_adapter.h"

namespace deviceapp {

class MockDeviceAdapter : public IDeviceAdapter {
    Q_OBJECT

public:
    explicit MockDeviceAdapter(QObject *parent = nullptr);

    void connectToDevice(const DeviceConnectionConfig &config) override;
    void disconnectFromDevice() override;
    void readStatusSnapshot() override;
    void applyScanSettings(const ScanSettings &settings) override;
    void applyTuneParameters(const TuneParameters &parameters) override;
    void applyDataProcessingSettings(const DataProcessingSettings &settings) override;
    void setSwitchState(InstrumentSwitch instrumentSwitch, bool on) override;
    void startScan(ScanMode mode) override;
    void stopScan() override;
    void calibrateMassAxis() override;
    void saveCurrentFrame() override;

private slots:
    void produceFrame();
    void updateAmbientState();

private:
    SpectrumFrame buildFrame() const;
    InstrumentStatus currentStatus() const;

    DeviceConnectionConfig connectionConfig_;
    ScanSettings scanSettings_;
    TuneParameters tuneParameters_;
    DataProcessingSettings dataProcessingSettings_;
    InstrumentStatus status_;
    SpectrumFrame lastFrame_;
    QTimer scanTimer_;
    QTimer statusTimer_;
    double phase_ = 0.0;
};

}  // namespace deviceapp
