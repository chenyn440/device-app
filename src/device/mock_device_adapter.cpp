#include "device/mock_device_adapter.h"

#include <QtMath>

namespace deviceapp {

MockDeviceAdapter::MockDeviceAdapter(QObject *parent) : IDeviceAdapter(parent) {
    status_.temperatures = {
        {"离子源", 180.0, 180.0, 3.0, true},
        {"四极杆", 45.0, 45.0, 2.0, true},
        {"腔体", 35.0, 35.0, 2.0, false},
    };
    for (InstrumentSwitch key : {InstrumentSwitch::ForePump, InstrumentSwitch::ForeValve, InstrumentSwitch::MolecularPump,
                                 InstrumentSwitch::InletValve, InstrumentSwitch::Filament, InstrumentSwitch::Multiplier}) {
        status_.switchStates.insert(key, false);
    }

    scanTimer_.setInterval(150);
    connect(&scanTimer_, &QTimer::timeout, this, &MockDeviceAdapter::produceFrame);
    statusTimer_.setInterval(1000);
    connect(&statusTimer_, &QTimer::timeout, this, &MockDeviceAdapter::updateAmbientState);
}

void MockDeviceAdapter::connectToDevice(const DeviceConnectionConfig &config) {
    connectionConfig_ = config;
    status_.connected = true;
    status_.lastError.clear();
    status_.vacuum.valuePa = 3.0e-5;
    emit connectionChanged(true);
    emit statusUpdated(currentStatus());
    statusTimer_.start();
}

void MockDeviceAdapter::disconnectFromDevice() {
    scanTimer_.stop();
    statusTimer_.stop();
    status_.connected = false;
    status_.scanning = false;
    emit connectionChanged(false);
    emit statusUpdated(currentStatus());
}

void MockDeviceAdapter::readStatusSnapshot() {
    emit statusUpdated(currentStatus());
}

void MockDeviceAdapter::applyScanSettings(const ScanSettings &settings) {
    scanSettings_ = settings;
}

void MockDeviceAdapter::applyTuneParameters(const TuneParameters &parameters) {
    tuneParameters_ = parameters;
}

void MockDeviceAdapter::applyDataProcessingSettings(const DataProcessingSettings &settings) {
    dataProcessingSettings_ = settings;
}

void MockDeviceAdapter::setSwitchState(InstrumentSwitch instrumentSwitch, bool on) {
    status_.switchStates[instrumentSwitch] = on;
    emit statusUpdated(currentStatus());
}

void MockDeviceAdapter::startScan(ScanMode mode) {
    if (!status_.connected) {
        emit errorOccurred("设备未连接");
        return;
    }
    scanSettings_.mode = mode;
    status_.scanning = true;
    emit statusUpdated(currentStatus());
    scanTimer_.start();
}

void MockDeviceAdapter::stopScan() {
    scanTimer_.stop();
    status_.scanning = false;
    emit statusUpdated(currentStatus());
}

void MockDeviceAdapter::calibrateMassAxis() {
    QTimer::singleShot(1200, this, [this]() {
        emit calibrationFinished(true, "质量轴校正完成");
    });
}

void MockDeviceAdapter::saveCurrentFrame() {
    emit frameReadyToPersist(lastFrame_);
}

void MockDeviceAdapter::produceFrame() {
    phase_ += 0.2;
    lastFrame_ = applyDataProcessing(buildFrame(), dataProcessingSettings_);
    emit frameUpdated(lastFrame_);
}

void MockDeviceAdapter::updateAmbientState() {
    status_.vacuum.valuePa = status_.scanning ? 4.0e-5 : 3.0e-5;
    for (int i = 0; i < status_.temperatures.size(); ++i) {
        auto &temperature = status_.temperatures[i];
        if (!temperature.connected) {
            continue;
        }
        temperature.current = temperature.target + qSin(phase_ + i) * (temperature.tolerance * 0.7);
    }
    emit statusUpdated(currentStatus());
}

SpectrumFrame MockDeviceAdapter::buildFrame() const {
    SpectrumFrame frame;
    frame.timestamp = QDateTime::currentDateTime();
    frame.scanMode = scanSettings_.mode;
    frame.detector = tuneParameters_.detector;

    QVector<double> centers = scanSettings_.mode == ScanMode::FullScan
        ? QVector<double>{18.0, 28.0, 44.0, 69.0, 91.0}
        : (scanSettings_.targetIons.isEmpty() ? QVector<double>{28.0} : scanSettings_.targetIons);

    for (int i = 0; i < 240; ++i) {
        const double mass = scanSettings_.massStart + (scanSettings_.massEnd - scanSettings_.massStart) * i / 239.0;
        double intensity = 0.0;
        for (int j = 0; j < centers.size(); ++j) {
            const double width = scanSettings_.mode == ScanMode::FullScan ? 1.8 : 0.5;
            const double amplitude = 1500.0 + 500.0 * qSin(phase_ + j);
            intensity += amplitude * qExp(-qPow((mass - centers[j]) / width, 2.0));
        }
        intensity += 60.0 * (1.0 + qSin(phase_ * 2.0 + mass / 8.0));
        frame.masses.append(mass);
        frame.intensities.append(intensity);
    }

    for (double center : centers) {
        frame.peaks.append({center, 1800.0 + 400.0 * qSin(phase_ + center)});
    }

    frame.parameterSnapshot = QJsonObject{
        {"scanSettings", toJson(scanSettings_)},
        {"tuneParameters", toJson(tuneParameters_)},
        {"dataProcessing", toJson(dataProcessingSettings_)},
        {"host", connectionConfig_.host},
        {"port", static_cast<int>(connectionConfig_.port)},
    };
    return frame;
}

InstrumentStatus MockDeviceAdapter::currentStatus() const {
    return status_;
}

}  // namespace deviceapp
