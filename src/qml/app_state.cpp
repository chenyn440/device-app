#include "qml/app_state.h"

#include <QtMath>

namespace deviceapp {
namespace {

bool parseInstrumentSwitchKey(const QString &switchKey, InstrumentSwitch *instrumentSwitch) {
    if (!instrumentSwitch) {
        return false;
    }
    const QString key = switchKey.trimmed().toLower();
    if (key == "forepump") {
        *instrumentSwitch = InstrumentSwitch::ForePump;
        return true;
    }
    if (key == "forevalve") {
        *instrumentSwitch = InstrumentSwitch::ForeValve;
        return true;
    }
    if (key == "molecularpump") {
        *instrumentSwitch = InstrumentSwitch::MolecularPump;
        return true;
    }
    if (key == "inletvalve") {
        *instrumentSwitch = InstrumentSwitch::InletValve;
        return true;
    }
    if (key == "filament") {
        *instrumentSwitch = InstrumentSwitch::Filament;
        return true;
    }
    if (key == "multiplier") {
        *instrumentSwitch = InstrumentSwitch::Multiplier;
        return true;
    }
    return false;
}

}  // namespace

AppState::AppState(QObject *parent) : QObject(parent) {
    connectionConfig_ = context_.connectionService->recentConnection();

    QObject::connect(context_.deviceAdapter.get(), &IDeviceAdapter::statusUpdated, static_cast<QObject *>(this),
                     [this](const InstrumentStatus &status) {
        status_ = status;
        context_.scanControlService->updateInstrumentStatus(status_);
        emit statusChanged();
    });

    QObject::connect(context_.deviceAdapter.get(), &IDeviceAdapter::frameUpdated, static_cast<QObject *>(this),
                     [this](const SpectrumFrame &frame) {
        latestFrame_ = frame;

        ticValue_ = 0.0;
        ricValue_ = 0.0;
        for (double v : latestFrame_.intensities) {
            ticValue_ += v;
            ricValue_ = qMax(ricValue_, v);
        }
        peak18_ = findPeak(latestFrame_, 18.0, 1.5);
        peak28_ = findPeak(latestFrame_, 28.0, 1.5);
        peak44_ = findPeak(latestFrame_, 44.0, 1.5);

        ricTrend_.append(QPointF(trendIndex_, ricValue_));
        ticTrend_.append(QPointF(trendIndex_, ticValue_));
        ++trendIndex_;
        if (ricTrend_.size() > maxTrendPoints_) {
            ricTrend_.removeFirst();
        }
        if (ticTrend_.size() > maxTrendPoints_) {
            ticTrend_.removeFirst();
        }

        updateSpectrumSeries();
        updateTrendSeries();
        emit frameChanged();
    });

    QObject::connect(context_.deviceAdapter.get(), &IDeviceAdapter::errorOccurred, static_cast<QObject *>(this),
                     [this](const QString &message) {
        status_.lastError = message;
        emit statusChanged();
        emit toastRequested(message);
    });

    context_.deviceAdapter->readStatusSnapshot();
}

void AppState::connectToDevice(const QString &host, int port) {
    DeviceConnectionConfig config;
    config.host = host.trimmed().isEmpty() ? "127.0.0.1" : host.trimmed();
    config.port = static_cast<quint16>(qBound(1, port, 65535));
    connectionConfig_ = config;
    emit connectionConfigChanged();
    context_.connectionService->connectToDevice(config);
}

void AppState::disconnectFromDevice() {
    context_.connectionService->disconnectFromDevice();
}

void AppState::startScan(double massStart, double massEnd) {
    ScanSettings settings;
    settings.mode = ScanMode::FullScan;
    settings.massStart = massStart;
    settings.massEnd = massEnd;
    settings.scanTimeMs = 500.0;
    settings.scanSpeed = 1000.0;
    settings.flybackTimeMs = 100.0;
    settings.sampleRateHz = 1000.0;
    settings.samplingPoints = 1024;

    QString error;
    if (!context_.scanControlService->canStartScan(settings, &error)) {
        emit toastRequested(error);
        return;
    }
    context_.scanControlService->startScan(settings);
}

void AppState::stopScan() {
    context_.scanControlService->stopScan();
}

void AppState::applyTune(double repellerVoltage, double lens1Voltage, double lens2Voltage) {
    TuneParameters p = context_.tuneService->currentParameters();
    p.repellerVoltage = repellerVoltage;
    p.lens1Voltage = lens1Voltage;
    p.lens2Voltage = lens2Voltage;
    context_.tuneService->applyTuneParameters(p);
    emit toastRequested("调谐参数已应用");
}

void AppState::calibrateMassAxis() {
    context_.scanControlService->calibrateMassAxis();
    emit toastRequested("已执行质量轴校正");
}

bool AppState::switchState(const QString &switchKey) const {
    InstrumentSwitch instrumentSwitch;
    if (!parseInstrumentSwitchKey(switchKey, &instrumentSwitch)) {
        return false;
    }
    return status_.switchStates.value(instrumentSwitch, false);
}

void AppState::setInstrumentSwitch(const QString &switchKey, bool on) {
    InstrumentSwitch instrumentSwitch;
    if (!parseInstrumentSwitchKey(switchKey, &instrumentSwitch)) {
        emit toastRequested("未知开关: " + switchKey);
        return;
    }
    context_.deviceAdapter->setSwitchState(instrumentSwitch, on);
    status_.switchStates[instrumentSwitch] = on;
    emit statusChanged();
    emit toastRequested(QString("%1: %2").arg(instrumentSwitchToString(instrumentSwitch), on ? "开启" : "关闭"));
}

void AppState::toggleInstrumentSwitch(const QString &switchKey) {
    const bool on = switchState(switchKey);
    setInstrumentSwitch(switchKey, !on);
}

void AppState::toggleDetectorMode() {
    TuneParameters p = context_.tuneService->currentParameters();
    p.detector = (p.detector == DetectorType::ElectronMultiplier) ? DetectorType::FaradayCup : DetectorType::ElectronMultiplier;
    context_.tuneService->applyTuneParameters(p);
    emit toastRequested(QString("检测器已切换为: %1").arg(detectorTypeToString(p.detector)));
}

void AppState::saveCurrentFrame() {
    if (!latestFrame_.timestamp.isValid()) {
        emit toastRequested("当前没有可保存的单帧数据");
        return;
    }
    const QString jsonPath = context_.persistenceService->saveFrame(latestFrame_);
    const QString csvPath = context_.persistenceService->exportFrameCsv(latestFrame_);
    emit toastRequested(QString("已保存 JSON: %1 | CSV: %2").arg(jsonPath, csvPath));
}

void AppState::refreshCharts() {
    updateSpectrumSeries();
    updateTrendSeries();
    emit toastRequested("谱图已刷新");
}

void AppState::bindSpectrumSeries(QObject *seriesObject) {
    spectrumSeries_ = qobject_cast<QXYSeries *>(seriesObject);
    updateSpectrumSeries();
}

void AppState::bindRicSeries(QObject *seriesObject) {
    ricSeries_ = qobject_cast<QXYSeries *>(seriesObject);
    updateTrendSeries();
}

void AppState::bindTicSeries(QObject *seriesObject) {
    ticSeries_ = qobject_cast<QXYSeries *>(seriesObject);
    updateTrendSeries();
}

void AppState::updateSpectrumSeries() {
    if (!spectrumSeries_) {
        return;
    }
    QVector<QPointF> points;
    points.reserve(qMin(latestFrame_.masses.size(), latestFrame_.intensities.size()));
    for (int i = 0; i < latestFrame_.masses.size() && i < latestFrame_.intensities.size(); ++i) {
        points.append(QPointF(latestFrame_.masses[i], latestFrame_.intensities[i]));
    }
    spectrumSeries_->replace(points);
}

void AppState::updateTrendSeries() {
    if (ricSeries_) {
        ricSeries_->replace(ricTrend_);
    }
    if (ticSeries_) {
        ticSeries_->replace(ticTrend_);
    }
}

QString AppState::runHint() const {
    if (!status_.connected) {
        return "设备未连接";
    }
    if (status_.scanning) {
        return "扫描进行中，参数已锁定";
    }
    return "当前未扫描，可编辑参数";
}

double AppState::findPeak(const SpectrumFrame &frame, double mass, double tolerance) {
    for (const PeakInfo &peak : frame.peaks) {
        if (qAbs(peak.mass - mass) <= tolerance) {
            return peak.intensity;
        }
    }
    return 0.0;
}

}  // namespace deviceapp
