#include "device/real_device_adapter.h"

namespace deviceapp {

RealDeviceAdapter::RealDeviceAdapter(QObject *parent) : IDeviceAdapter(parent) {}

void RealDeviceAdapter::connectToDevice(const DeviceConnectionConfig &) {
    emit errorOccurred("真实设备协议尚未接入");
}

void RealDeviceAdapter::disconnectFromDevice() {}

void RealDeviceAdapter::readStatusSnapshot() {}

void RealDeviceAdapter::applyScanSettings(const ScanSettings &) {}

void RealDeviceAdapter::applyTuneParameters(const TuneParameters &) {}

void RealDeviceAdapter::applyDataProcessingSettings(const DataProcessingSettings &) {}

void RealDeviceAdapter::setSwitchState(InstrumentSwitch, bool) {}

void RealDeviceAdapter::startScan(ScanMode) {
    emit errorOccurred("真实设备协议尚未接入");
}

void RealDeviceAdapter::stopScan() {}

void RealDeviceAdapter::calibrateMassAxis() {
    emit errorOccurred("真实设备协议尚未接入");
}

void RealDeviceAdapter::saveCurrentFrame() {
    emit errorOccurred("真实设备协议尚未接入");
}

}  // namespace deviceapp
