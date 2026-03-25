#include "core/types.h"

#include <QJsonDocument>

namespace deviceapp {

bool VacuumStatus::isReady() const {
    return valuePa <= thresholdPa;
}

bool TemperatureStatus::isWithinTolerance() const {
    return connected && qAbs(current - target) <= tolerance;
}

bool ScanSettings::isValid(QString *errorMessage) const {
    if (mode == ScanMode::FullScan && massStart >= massEnd) {
        if (errorMessage) {
            *errorMessage = "质量范围无效";
        }
        return false;
    }
    if (mode == ScanMode::SelectedIon && targetIons.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "选择离子扫描至少需要一个目标离子";
        }
        return false;
    }
    if (mode == ScanMode::SelectedIon && !targetVoltages.isEmpty() && targetVoltages.size() != targetIons.size()) {
        if (errorMessage) {
            *errorMessage = "目标离子与目标电压数量不一致";
        }
        return false;
    }
    if (scanTimeMs <= 0.0) {
        if (errorMessage) {
            *errorMessage = "扫描时间必须大于 0";
        }
        return false;
    }
    if (mode == ScanMode::FullScan && useTargetVoltage && voltageRangeMin > voltageRangeMax) {
        if (errorMessage) {
            *errorMessage = "电压范围无效";
        }
        return false;
    }
    if (mode == ScanMode::SelectedIon && targetDwellTimeMs <= 0.0) {
        if (errorMessage) {
            *errorMessage = "驻留时间必须大于 0";
        }
        return false;
    }
    if (mode == ScanMode::SelectedIon && targetPeakWidth <= 0.0) {
        if (errorMessage) {
            *errorMessage = "目标峰宽度必须大于 0";
        }
        return false;
    }
    return true;
}

QString scanModeToString(ScanMode mode) {
    return mode == ScanMode::FullScan ? "全扫描" : "选择离子扫描";
}

QString detectorTypeToString(DetectorType detector) {
    return detector == DetectorType::ElectronMultiplier ? "电子倍增器" : "法拉第筒";
}

QString instrumentSwitchToString(InstrumentSwitch instrumentSwitch) {
    switch (instrumentSwitch) {
    case InstrumentSwitch::ForePump:
        return "前级泵";
    case InstrumentSwitch::ForeValve:
        return "前级阀";
    case InstrumentSwitch::MolecularPump:
        return "分子泵";
    case InstrumentSwitch::InletValve:
        return "进样阀";
    case InstrumentSwitch::Filament:
        return "灯丝";
    case InstrumentSwitch::Multiplier:
        return "倍增器";
    }
    return "未知";
}

QJsonObject toJson(const DeviceConnectionConfig &config) {
    return {{"host", config.host}, {"port", static_cast<int>(config.port)}};
}

QJsonObject toJson(const ScanSettings &settings) {
    QJsonArray targets;
    for (double target : settings.targetIons) {
        targets.append(target);
    }
    QJsonArray targetVoltages;
    for (double voltage : settings.targetVoltages) {
        targetVoltages.append(voltage);
    }
    return {
        {"mode", settings.mode == ScanMode::FullScan ? "full" : "sim"},
        {"massStart", settings.massStart},
        {"massEnd", settings.massEnd},
        {"scanSpeed", settings.scanSpeed},
        {"scanTimeMs", settings.scanTimeMs},
        {"scanTimePreset", settings.scanTimePreset},
        {"flybackTimeMs", settings.flybackTimeMs},
        {"sampleRateHz", settings.sampleRateHz},
        {"sampleRateText", settings.sampleRateText},
        {"samplingPoints", settings.samplingPoints},
        {"useMassAxisCalibration", settings.useMassAxisCalibration},
        {"massAxisSlope", settings.massAxisSlope},
        {"massAxisOffset", settings.massAxisOffset},
        {"targetDwellTimeMs", settings.targetDwellTimeMs},
        {"targetPeakWidth", settings.targetPeakWidth},
        {"rampVoltage", settings.rampVoltage},
        {"useTargetVoltage", settings.useTargetVoltage},
        {"voltageRangeMin", settings.voltageRangeMin},
        {"voltageRangeMax", settings.voltageRangeMax},
        {"targetIons", targets},
        {"targetVoltages", targetVoltages},
    };
}

QJsonObject toJson(const TuneParameters &parameters) {
    return {
        {"detector", parameters.detector == DetectorType::ElectronMultiplier ? "em" : "fc"},
        {"repellerVoltage", parameters.repellerVoltage},
        {"lens1Voltage", parameters.lens1Voltage},
        {"lens2Voltage", parameters.lens2Voltage},
        {"highMassCompensation", parameters.highMassCompensation},
        {"lowMassCompensation", parameters.lowMassCompensation},
        {"multiplierVoltage", parameters.multiplierVoltage},
        {"rodVoltage", parameters.rodVoltage},
        {"eVoltage", parameters.eVoltage},
        {"electronEnergy", parameters.electronEnergy},
        {"filamentCurrent", parameters.filamentCurrent},
        {"outerDeflectionLensVoltage", parameters.outerDeflectionLensVoltage},
        {"innerDeflectionLensVoltage", parameters.innerDeflectionLensVoltage},
        {"preQuadrupoleFrontVoltage", parameters.preQuadrupoleFrontVoltage},
        {"preQuadrupoleRearVoltage", parameters.preQuadrupoleRearVoltage},
    };
}

QJsonObject toJson(const MonitorMethod &method) {
    return {
        {"name", method.name},
        {"detector", method.detector == DetectorType::ElectronMultiplier ? "em" : "fc"},
        {"dwellTimeMs", method.dwellTimeMs},
        {"scanSettings", toJson(method.scanSettings)},
    };
}

QJsonObject toJson(const TemperatureStatus &status) {
    return {
        {"name", status.name},
        {"current", status.current},
        {"target", status.target},
        {"tolerance", status.tolerance},
        {"connected", status.connected},
    };
}

QJsonObject toJson(const PeakInfo &peak) {
    return {{"mass", peak.mass}, {"intensity", peak.intensity}};
}

QJsonObject toJson(const SpectrumFrame &frame) {
    QJsonArray masses;
    for (double mass : frame.masses) {
        masses.append(mass);
    }
    QJsonArray intensities;
    for (double intensity : frame.intensities) {
        intensities.append(intensity);
    }
    QJsonArray peaks;
    for (const PeakInfo &peak : frame.peaks) {
        peaks.append(toJson(peak));
    }
    return {
        {"timestamp", frame.timestamp.toString(Qt::ISODate)},
        {"scanMode", frame.scanMode == ScanMode::FullScan ? "full" : "sim"},
        {"detector", frame.detector == DetectorType::ElectronMultiplier ? "em" : "fc"},
        {"masses", masses},
        {"intensities", intensities},
        {"peaks", peaks},
        {"parameterSnapshot", frame.parameterSnapshot},
    };
}

QJsonObject toJson(const DataProcessingSettings &settings) {
    return {
        {"smoothingEnabled", settings.smoothingEnabled},
        {"smoothingWindow", settings.smoothingWindow},
        {"baselineLambda", settings.baselineLambda},
        {"baselineAsymmetry", settings.baselineAsymmetry},
    };
}

QJsonObject toJson(const SystemSettings &settings) {
    return {
        {"rfddsValue", settings.rfddsValue},
        {"systemSerialNumber", settings.systemSerialNumber},
        {"systemModel", settings.systemModel},
    };
}

QJsonObject toJson(const ChartSettings &settings) {
    return {
        {"xAxisMode", settings.xAxisMode},
        {"disableAutoRestoreFullScale", settings.disableAutoRestoreFullScale},
        {"fixedYRange", settings.fixedYRange},
        {"yMin", settings.yMin},
        {"yMax", settings.yMax},
        {"peakCount", settings.peakCount},
        {"showRawData", settings.showRawData},
        {"showPersistence", settings.showPersistence},
        {"showHalfPeakWidth", settings.showHalfPeakWidth},
        {"showTicChart", settings.showTicChart},
    };
}

QJsonObject toJson(const InstrumentControlSettings &settings) {
    return {
        {"forePumpEnabled", settings.forePumpEnabled},
        {"foreValveEnabled", settings.foreValveEnabled},
        {"molecularPumpEnabled", settings.molecularPumpEnabled},
        {"inletValveEnabled", settings.inletValveEnabled},
        {"filamentEnabled", settings.filamentEnabled},
        {"multiplierEnabled", settings.multiplierEnabled},
        {"targetSourceTemperature", settings.targetSourceTemperature},
        {"targetChamberTemperature", settings.targetChamberTemperature},
    };
}

DeviceConnectionConfig connectionConfigFromJson(const QJsonObject &json) {
    DeviceConnectionConfig config;
    config.host = json.value("host").toString(config.host);
    config.port = static_cast<quint16>(json.value("port").toInt(config.port));
    return config;
}

ScanSettings scanSettingsFromJson(const QJsonObject &json) {
    ScanSettings settings;
    settings.mode = json.value("mode").toString() == "sim" ? ScanMode::SelectedIon : ScanMode::FullScan;
    settings.massStart = json.value("massStart").toDouble(settings.massStart);
    settings.massEnd = json.value("massEnd").toDouble(settings.massEnd);
    settings.scanSpeed = json.value("scanSpeed").toDouble(settings.scanSpeed);
    settings.scanTimeMs = json.value("scanTimeMs").toDouble(settings.scanTimeMs);
    settings.scanTimePreset = json.value("scanTimePreset").toString(settings.scanTimePreset);
    settings.flybackTimeMs = json.value("flybackTimeMs").toDouble(settings.flybackTimeMs);
    settings.sampleRateHz = json.value("sampleRateHz").toDouble(settings.sampleRateHz);
    settings.sampleRateText = json.value("sampleRateText").toString(settings.sampleRateText);
    settings.samplingPoints = json.value("samplingPoints").toInt(settings.samplingPoints);
    settings.useMassAxisCalibration = json.value("useMassAxisCalibration").toBool(settings.useMassAxisCalibration);
    settings.massAxisSlope = json.value("massAxisSlope").toDouble(settings.massAxisSlope);
    settings.massAxisOffset = json.value("massAxisOffset").toDouble(settings.massAxisOffset);
    settings.targetDwellTimeMs = json.value("targetDwellTimeMs").toDouble(settings.targetDwellTimeMs);
    settings.targetPeakWidth = json.value("targetPeakWidth").toDouble(settings.targetPeakWidth);
    settings.rampVoltage = json.value("rampVoltage").toDouble(settings.rampVoltage);
    settings.useTargetVoltage = json.value("useTargetVoltage").toBool(settings.useTargetVoltage);
    settings.voltageRangeMin = json.value("voltageRangeMin").toDouble(settings.voltageRangeMin);
    settings.voltageRangeMax = json.value("voltageRangeMax").toDouble(settings.voltageRangeMax);
    for (const QJsonValue &value : json.value("targetIons").toArray()) {
        settings.targetIons.append(value.toDouble());
    }
    for (const QJsonValue &value : json.value("targetVoltages").toArray()) {
        settings.targetVoltages.append(value.toDouble());
    }
    return settings;
}

TuneParameters tuneParametersFromJson(const QJsonObject &json) {
    TuneParameters parameters;
    parameters.detector = json.value("detector").toString() == "fc" ? DetectorType::FaradayCup : DetectorType::ElectronMultiplier;
    parameters.repellerVoltage = json.value("repellerVoltage").toDouble(parameters.repellerVoltage);
    parameters.lens1Voltage = json.value("lens1Voltage").toDouble(parameters.lens1Voltage);
    parameters.lens2Voltage = json.value("lens2Voltage").toDouble(parameters.lens2Voltage);
    parameters.highMassCompensation = json.value("highMassCompensation").toDouble(parameters.highMassCompensation);
    parameters.lowMassCompensation = json.value("lowMassCompensation").toDouble(parameters.lowMassCompensation);
    parameters.multiplierVoltage = json.value("multiplierVoltage").toDouble(
        json.value("detectorVoltage").toDouble(parameters.multiplierVoltage));
    parameters.rodVoltage = json.value("rodVoltage").toDouble(parameters.rodVoltage);
    parameters.eVoltage = json.value("eVoltage").toDouble(parameters.eVoltage);
    parameters.electronEnergy = json.value("electronEnergy").toDouble(parameters.electronEnergy);
    parameters.filamentCurrent = json.value("filamentCurrent").toDouble(
        json.value("emissionCurrent").toDouble(parameters.filamentCurrent));
    parameters.outerDeflectionLensVoltage = json.value("outerDeflectionLensVoltage").toDouble(parameters.outerDeflectionLensVoltage);
    parameters.innerDeflectionLensVoltage = json.value("innerDeflectionLensVoltage").toDouble(parameters.innerDeflectionLensVoltage);
    parameters.preQuadrupoleFrontVoltage = json.value("preQuadrupoleFrontVoltage").toDouble(parameters.preQuadrupoleFrontVoltage);
    parameters.preQuadrupoleRearVoltage = json.value("preQuadrupoleRearVoltage").toDouble(parameters.preQuadrupoleRearVoltage);
    return parameters;
}

MonitorMethod monitorMethodFromJson(const QJsonObject &json) {
    MonitorMethod method;
    method.name = json.value("name").toString(method.name);
    method.detector = json.value("detector").toString() == "fc" ? DetectorType::FaradayCup : DetectorType::ElectronMultiplier;
    method.dwellTimeMs = json.value("dwellTimeMs").toDouble(method.dwellTimeMs);
    method.scanSettings = scanSettingsFromJson(json.value("scanSettings").toObject());
    return method;
}

SpectrumFrame spectrumFrameFromJson(const QJsonObject &json) {
    SpectrumFrame frame;
    frame.timestamp = QDateTime::fromString(json.value("timestamp").toString(), Qt::ISODate);
    frame.scanMode = json.value("scanMode").toString() == "sim" ? ScanMode::SelectedIon : ScanMode::FullScan;
    frame.detector = json.value("detector").toString() == "fc" ? DetectorType::FaradayCup : DetectorType::ElectronMultiplier;
    for (const QJsonValue &value : json.value("masses").toArray()) {
        frame.masses.append(value.toDouble());
    }
    for (const QJsonValue &value : json.value("intensities").toArray()) {
        frame.intensities.append(value.toDouble());
    }
    for (const QJsonValue &value : json.value("peaks").toArray()) {
        const QJsonObject peakJson = value.toObject();
        frame.peaks.append({peakJson.value("mass").toDouble(), peakJson.value("intensity").toDouble()});
    }
    frame.parameterSnapshot = json.value("parameterSnapshot").toObject();
    return frame;
}

DataProcessingSettings dataProcessingSettingsFromJson(const QJsonObject &json) {
    DataProcessingSettings settings;
    settings.smoothingEnabled = json.value("smoothingEnabled").toBool(settings.smoothingEnabled);
    settings.smoothingWindow = json.value("smoothingWindow").toInt(settings.smoothingWindow);
    settings.baselineLambda = json.value("baselineLambda").toDouble(settings.baselineLambda);
    settings.baselineAsymmetry = json.value("baselineAsymmetry").toDouble(settings.baselineAsymmetry);
    return settings;
}

SystemSettings systemSettingsFromJson(const QJsonObject &json) {
    SystemSettings settings;
    settings.rfddsValue = json.value("rfddsValue").toString(settings.rfddsValue);
    settings.systemSerialNumber = json.value("systemSerialNumber").toString(settings.systemSerialNumber);
    settings.systemModel = json.value("systemModel").toString(settings.systemModel);
    return settings;
}

ChartSettings chartSettingsFromJson(const QJsonObject &json) {
    ChartSettings settings;
    settings.xAxisMode = json.value("xAxisMode").toString(settings.xAxisMode);
    settings.disableAutoRestoreFullScale = json.value("disableAutoRestoreFullScale").toBool(settings.disableAutoRestoreFullScale);
    settings.fixedYRange = json.value("fixedYRange").toBool(settings.fixedYRange);
    settings.yMin = json.value("yMin").toDouble(settings.yMin);
    settings.yMax = json.value("yMax").toDouble(settings.yMax);
    settings.peakCount = json.value("peakCount").toInt(settings.peakCount);
    settings.showRawData = json.value("showRawData").toBool(settings.showRawData);
    settings.showPersistence = json.value("showPersistence").toBool(settings.showPersistence);
    settings.showHalfPeakWidth = json.value("showHalfPeakWidth").toBool(settings.showHalfPeakWidth);
    settings.showTicChart = json.value("showTicChart").toBool(settings.showTicChart);
    if (json.contains("showPeaks") && !json.contains("peakCount")) {
        settings.peakCount = json.value("showPeaks").toBool() ? 15 : 0;
    }
    if (json.contains("autoScaleY") && !json.contains("fixedYRange")) {
        settings.fixedYRange = !json.value("autoScaleY").toBool(true);
    }
    if (json.contains("xAxisLabel") && !json.contains("xAxisMode")) {
        const QString label = json.value("xAxisLabel").toString().toLower();
        if (label.contains("volt")) {
            settings.xAxisMode = "voltage";
        } else if (label.contains("time")) {
            settings.xAxisMode = "time";
        } else if (label.contains("point")) {
            settings.xAxisMode = "point";
        } else {
            settings.xAxisMode = "mass";
        }
    }
    return settings;
}

InstrumentControlSettings instrumentControlSettingsFromJson(const QJsonObject &json) {
    InstrumentControlSettings settings;
    settings.forePumpEnabled = json.value("forePumpEnabled").toBool(settings.forePumpEnabled);
    settings.foreValveEnabled = json.value("foreValveEnabled").toBool(settings.foreValveEnabled);
    settings.molecularPumpEnabled = json.value("molecularPumpEnabled").toBool(settings.molecularPumpEnabled);
    settings.inletValveEnabled = json.value("inletValveEnabled").toBool(settings.inletValveEnabled);
    settings.filamentEnabled = json.value("filamentEnabled").toBool(settings.filamentEnabled);
    settings.multiplierEnabled = json.value("multiplierEnabled").toBool(settings.multiplierEnabled);
    settings.targetSourceTemperature = json.value("targetSourceTemperature").toDouble(settings.targetSourceTemperature);
    settings.targetChamberTemperature = json.value("targetChamberTemperature").toDouble(settings.targetChamberTemperature);
    return settings;
}

SpectrumFrame applyDataProcessing(const SpectrumFrame &frame, const DataProcessingSettings &settings) {
    SpectrumFrame processed = frame;
    if (processed.intensities.isEmpty()) {
        return processed;
    }

    QVector<double> filtered = processed.intensities;
    if (settings.smoothingEnabled) {
        QVector<double> smoothed = filtered;
        const int halfWindow = qMax(0, settings.smoothingWindow / 2);
        for (int i = 0; i < filtered.size(); ++i) {
            const int begin = qMax(0, i - halfWindow);
            const int end = qMin(filtered.size() - 1, i + halfWindow);
            double sum = 0.0;
            int count = 0;
            for (int j = begin; j <= end; ++j) {
                sum += filtered[j];
                ++count;
            }
            smoothed[i] = count > 0 ? sum / count : filtered[i];
        }
        filtered = smoothed;
    }

    const double baselineFactor = qBound(0.0, settings.baselineAsymmetry, 1.0);
    const double baselineScale = qLn(1.0 + qMax(1.0, settings.baselineLambda)) * baselineFactor;
    double baseline = *std::min_element(filtered.begin(), filtered.end());
    baseline *= baselineScale;
    for (double &value : filtered) {
        value = qMax(0.0, value - baseline);
    }

    processed.intensities = filtered;
    processed.peaks.clear();
    for (int i = 1; i + 1 < filtered.size(); ++i) {
        if (filtered[i] > filtered[i - 1] && filtered[i] >= filtered[i + 1] && filtered[i] > baseline + 100.0) {
            processed.peaks.append({processed.masses.value(i), filtered[i]});
        }
    }
    return processed;
}

}  // namespace deviceapp
