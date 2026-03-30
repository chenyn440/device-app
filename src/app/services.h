#pragma once

#include <QObject>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QTimer>
#include <QUrl>

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
    QString saveSnapshotJson(const QJsonObject &snapshot, const QString &baseName = QString()) const;
    QString saveReportText(const QString &report, const QString &baseName = QString()) const;

private:
    FrameRepository *frameRepository_;
};

struct AiResult {
    bool success = false;
    QString message;
    QString content;
    QJsonObject payload;
};

class AiAssistantService : public QObject {
    Q_OBJECT

public:
    explicit AiAssistantService(QObject *parent = nullptr);

    QString modelTag() const { return modelTag_; }
    QString baseUrl() const { return baseUrl_; }
    QJsonObject providerConfig() const;
    bool applyProviderConfig(const QJsonObject &config, QString *errorMessage = nullptr);

    AiResult modelStatus();
    AiResult pullModel(const QString &modelTag = QString());
    AiResult summarizeTune(const InstrumentStatus &status, const SpectrumFrame &frame,
                           const TuneParameters &tune, const ScanSettings &scan);
    AiResult troubleshootTune(const InstrumentStatus &status, const SpectrumFrame &frame,
                              const TuneParameters &tune, const ScanSettings &scan,
                              const QString &question);
    bool startGenerateStream(const QString &prompt, const QString &modelTag = QString());
    void cancelStream();
    bool streamRunning() const { return streamRunning_; }

signals:
    void streamChunk(const QString &chunk);
    void streamCompleted(const deviceapp::AiResult &result);

private:
    enum class StreamFormat {
        OllamaNdjson,
        Sse
    };

    bool useCloudProvider() const;
    bool useClaudeProvider() const;
    AiResult requestZhipuGenerate(const QString &prompt, const QString &modelTag);
    AiResult requestClaudeGenerate(const QString &prompt, const QString &modelTag);
    bool startZhipuGenerateStream(const QString &prompt, const QString &modelTag);
    bool startClaudeGenerateStream(const QString &prompt, const QString &modelTag);
    QString resolveModelTag(const QString &modelTag) const;
    static QString extractZhipuContent(const QJsonObject &obj);
    static QString extractZhipuError(const QJsonObject &obj);
    static QString extractClaudeContent(const QJsonObject &obj);
    static QString extractClaudeError(const QJsonObject &obj);

    AiResult requestJson(const QUrl &url, const QJsonObject &payload, const QByteArray &method);
    AiResult requestGenerate(const QString &prompt, const QString &modelTag);
    void finalizeStreamResult(const AiResult &result);
    void handleStreamReadyRead();
    QJsonObject buildTuneContext(const InstrumentStatus &status, const SpectrumFrame &frame,
                                 const TuneParameters &tune, const ScanSettings &scan) const;

    QNetworkAccessManager network_;
    QString provider_;
    QString baseUrl_;
    QString modelTag_;
    QString zhipuApiKey_;
    QString zhipuBaseUrl_;
    int timeoutMs_ = 120000;
    QPointer<QNetworkReply> streamReply_;
    QTimer streamTimeout_;
    QByteArray streamBuffer_;
    QString streamAccumulated_;
    StreamFormat streamFormat_ = StreamFormat::OllamaNdjson;
    bool streamRunning_ = false;
    bool streamCanceled_ = false;
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
    QJsonObject aiChatState() const;
    void saveAiChatState(const QJsonObject &state) const;
    QJsonObject aiProviderConfig() const;
    void saveAiProviderConfig(const QJsonObject &config) const;
    QJsonObject aiSummaryHistory() const;
    void saveAiSummaryHistory(const QJsonObject &history) const;

private:
    SettingsRepository *settingsRepository_;
};

}  // namespace deviceapp
