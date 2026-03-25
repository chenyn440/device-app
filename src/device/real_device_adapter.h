#pragma once

#include "device/device_adapter.h"

namespace deviceapp {

class RealDeviceAdapter : public IDeviceAdapter {
    Q_OBJECT

public:
    explicit RealDeviceAdapter(QObject *parent = nullptr);

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
};

}  // namespace deviceapp
