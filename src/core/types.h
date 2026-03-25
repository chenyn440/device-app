#pragma once

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QVector>

namespace deviceapp {

Q_NAMESPACE

enum class ScanMode {
    FullScan,
    SelectedIon
};
Q_ENUM_NS(ScanMode)

enum class DetectorType {
    ElectronMultiplier,
    FaradayCup
};
Q_ENUM_NS(DetectorType)

enum class InstrumentSwitch {
    ForePump,
    ForeValve,
    MolecularPump,
    InletValve,
    Filament,
    Multiplier
};
Q_ENUM_NS(InstrumentSwitch)

struct DeviceConnectionConfig {
    QString host = "127.0.0.1";
    quint16 port = 9000;
};

struct VacuumStatus {
    double valuePa = 1.0e-5;
    double thresholdPa = 5.0e-5;

    bool isReady() const;
};

struct TemperatureStatus {
    QString name;
    double current = 0.0;
    double target = 0.0;
    double tolerance = 0.0;
    bool connected = true;

    bool isWithinTolerance() const;
};

struct PeakInfo {
    double mass = 0.0;
    double intensity = 0.0;
};

struct ScanSettings {
    ScanMode mode = ScanMode::FullScan;
    double massStart = 1.0;
    double massEnd = 200.0;
    double scanSpeed = 1000.0;
    double scanTimeMs = 500.0;
    QString scanTimePreset = "自定义";
    double flybackTimeMs = 100.0;
    double sampleRateHz = 1000.0;
    QString sampleRateText = "20K";
    int samplingPoints = 1024;
    bool useMassAxisCalibration = false;
    double massAxisSlope = 1.0;
    double massAxisOffset = 0.0;
    double targetDwellTimeMs = 20.0;
    double targetPeakWidth = 0.8;
    double rampVoltage = 0.0;
    bool useTargetVoltage = false;
    double voltageRangeMin = 0.0;
    double voltageRangeMax = 1000.0;
    QVector<double> targetIons;
    QVector<double> targetVoltages;

    bool isValid(QString *errorMessage = nullptr) const;
};

struct TuneParameters {
    DetectorType detector = DetectorType::ElectronMultiplier;
    double repellerVoltage = 12.0;
    double lens1Voltage = 6.0;
    double lens2Voltage = 10.0;
    double highMassCompensation = 0.0;
    double lowMassCompensation = 0.0;
    double multiplierVoltage = 1200.0;
    double rodVoltage = 45.0;
    double eVoltage = 0.0;
    double electronEnergy = 70.0;
    double filamentCurrent = 80.0;
    double outerDeflectionLensVoltage = 0.0;
    double innerDeflectionLensVoltage = 0.0;
    double preQuadrupoleFrontVoltage = 0.0;
    double preQuadrupoleRearVoltage = 0.0;
};

struct MonitorMethod {
    QString name = "Default";
    ScanSettings scanSettings;
    DetectorType detector = DetectorType::ElectronMultiplier;
    double dwellTimeMs = 20.0;
};

struct InstrumentStatus {
    bool connected = false;
    bool scanning = false;
    VacuumStatus vacuum;
    QList<TemperatureStatus> temperatures;
    QMap<InstrumentSwitch, bool> switchStates;
    QString lastError;
};

struct SpectrumFrame {
    QDateTime timestamp;
    ScanMode scanMode = ScanMode::FullScan;
    DetectorType detector = DetectorType::ElectronMultiplier;
    QVector<double> masses;
    QVector<double> intensities;
    QVector<PeakInfo> peaks;
    QJsonObject parameterSnapshot;
};

struct DataProcessingSettings {
    bool smoothingEnabled = true;
    int smoothingWindow = 5;
    double baselineLambda = 1000.0;
    double baselineAsymmetry = 0.01;
};

struct SystemSettings {
    QString rfddsValue = "0";
    QString systemSerialNumber = "";
    QString systemModel = "QMS-100";
};

struct ChartSettings {
    QString xAxisMode = "mass";
    bool disableAutoRestoreFullScale = true;
    bool fixedYRange = false;
    double yMin = 0.0;
    double yMax = 1000.0;
    int peakCount = 15;
    bool showRawData = false;
    bool showPersistence = false;
    bool showHalfPeakWidth = false;
    bool showTicChart = false;
};

struct InstrumentControlSettings {
    bool forePumpEnabled = false;
    bool foreValveEnabled = false;
    bool molecularPumpEnabled = false;
    bool inletValveEnabled = false;
    bool filamentEnabled = false;
    bool multiplierEnabled = false;
    double targetSourceTemperature = 180.0;
    double targetChamberTemperature = 35.0;
};

QString scanModeToString(ScanMode mode);
QString detectorTypeToString(DetectorType detector);
QString instrumentSwitchToString(InstrumentSwitch instrumentSwitch);

QJsonObject toJson(const DeviceConnectionConfig &config);
QJsonObject toJson(const ScanSettings &settings);
QJsonObject toJson(const TuneParameters &parameters);
QJsonObject toJson(const MonitorMethod &method);
QJsonObject toJson(const TemperatureStatus &status);
QJsonObject toJson(const PeakInfo &peak);
QJsonObject toJson(const SpectrumFrame &frame);
QJsonObject toJson(const DataProcessingSettings &settings);
QJsonObject toJson(const SystemSettings &settings);
QJsonObject toJson(const ChartSettings &settings);
QJsonObject toJson(const InstrumentControlSettings &settings);

DeviceConnectionConfig connectionConfigFromJson(const QJsonObject &json);
ScanSettings scanSettingsFromJson(const QJsonObject &json);
TuneParameters tuneParametersFromJson(const QJsonObject &json);
MonitorMethod monitorMethodFromJson(const QJsonObject &json);
SpectrumFrame spectrumFrameFromJson(const QJsonObject &json);
DataProcessingSettings dataProcessingSettingsFromJson(const QJsonObject &json);
SystemSettings systemSettingsFromJson(const QJsonObject &json);
ChartSettings chartSettingsFromJson(const QJsonObject &json);
InstrumentControlSettings instrumentControlSettingsFromJson(const QJsonObject &json);
SpectrumFrame applyDataProcessing(const SpectrumFrame &frame, const DataProcessingSettings &settings);

}  // namespace deviceapp
