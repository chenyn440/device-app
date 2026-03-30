#pragma once

#include <QObject>
#include <QPointF>
#include <QPointer>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

#include <QtCharts/QXYSeries>

#include "app/application_context.h"

namespace deviceapp {

class AppState : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY statusChanged)
    Q_PROPERTY(bool scanning READ scanning NOTIFY statusChanged)
    Q_PROPERTY(double vacuumPa READ vacuumPa NOTIFY statusChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY statusChanged)
    Q_PROPERTY(int spectrumPointCount READ spectrumPointCount NOTIFY frameChanged)
    Q_PROPERTY(double ticValue READ ticValue NOTIFY frameChanged)
    Q_PROPERTY(double ricValue READ ricValue NOTIFY frameChanged)
    Q_PROPERTY(QString connectionHost READ connectionHost NOTIFY connectionConfigChanged)
    Q_PROPERTY(int connectionPort READ connectionPort NOTIFY connectionConfigChanged)
    Q_PROPERTY(double peak18 READ peak18 NOTIFY frameChanged)
    Q_PROPERTY(double peak28 READ peak28 NOTIFY frameChanged)
    Q_PROPERTY(double peak44 READ peak44 NOTIFY frameChanged)
    Q_PROPERTY(QString runHint READ runHint NOTIFY statusChanged)
    Q_PROPERTY(bool aiRunning READ aiRunning NOTIFY aiRunningChanged)

public:
    explicit AppState(QObject *parent = nullptr);

    bool connected() const { return status_.connected; }
    bool scanning() const { return status_.scanning; }
    double vacuumPa() const { return status_.vacuum.valuePa; }
    QString lastError() const { return status_.lastError; }
    int spectrumPointCount() const { return latestFrame_.masses.size(); }
    double ticValue() const { return ticValue_; }
    double ricValue() const { return ricValue_; }
    QString connectionHost() const { return connectionConfig_.host; }
    int connectionPort() const { return connectionConfig_.port; }
    double peak18() const { return peak18_; }
    double peak28() const { return peak28_; }
    double peak44() const { return peak44_; }
    QString runHint() const;
    bool aiRunning() const { return aiRunning_; }

    Q_INVOKABLE void connectToDevice(const QString &host, int port);
    Q_INVOKABLE void disconnectFromDevice();
    Q_INVOKABLE void startScan(double massStart, double massEnd);
    Q_INVOKABLE void stopScan();
    Q_INVOKABLE void applyTune(double repellerVoltage, double lens1Voltage, double lens2Voltage);
    Q_INVOKABLE void calibrateMassAxis();
    Q_INVOKABLE bool switchState(const QString &switchKey) const;
    Q_INVOKABLE void setInstrumentSwitch(const QString &switchKey, bool on);
    Q_INVOKABLE void toggleInstrumentSwitch(const QString &switchKey);
    Q_INVOKABLE void toggleDetectorMode();
    Q_INVOKABLE void saveCurrentFrame();
    Q_INVOKABLE void refreshCharts();
    Q_INVOKABLE void bindSpectrumSeries(QObject *seriesObject);
    Q_INVOKABLE void bindRicSeries(QObject *seriesObject);
    Q_INVOKABLE void bindTicSeries(QObject *seriesObject);
    Q_INVOKABLE QVariantList webChartPoints(const QString &chartName) const;
    Q_INVOKABLE QVariantMap aiModelStatus();
    Q_INVOKABLE QVariantMap aiProviderConfig();
    Q_INVOKABLE QVariantMap aiSaveProviderConfig(const QVariantMap &config);
    Q_INVOKABLE QVariantMap aiPullModel(const QString &model);
    Q_INVOKABLE QVariantMap aiTuneSummary();
    Q_INVOKABLE QVariantMap aiTuneTroubleshoot(const QString &question);
    Q_INVOKABLE QVariantMap aiTuneExport(const QString &question, bool includeTroubleshoot);
    Q_INVOKABLE QVariantMap aiSummaryHistory();
    Q_INVOKABLE QVariantMap aiSummaryAppend(const QVariantMap &entry);
    Q_INVOKABLE QVariantMap aiSummaryExportText(const QString &summary, const QString &model, const QString &createdAt);
    Q_INVOKABLE bool aiTuneSummaryAsync();
    Q_INVOKABLE bool aiTuneTroubleshootAsync(const QString &question);
    Q_INVOKABLE bool aiTuneExportAsync(const QString &question, bool includeTroubleshoot);
    Q_INVOKABLE void aiCancel();
    Q_INVOKABLE QVariantMap aiChatBootstrap();
    Q_INVOKABLE QVariantMap aiChatNewSession();
    Q_INVOKABLE QVariantMap aiChatSwitchSession(const QString &sessionId);
    Q_INVOKABLE QVariantMap aiChatSendAsync(const QString &message, bool useDeviceContext);
    Q_INVOKABLE QVariantMap aiChatClearCurrentSession();

signals:
    void statusChanged();
    void frameChanged();
    void connectionConfigChanged();
    void toastRequested(const QString &message);
    void aiRunningChanged();
    void aiStreamChunk(const QString &chunk);
    void aiRequestFinished(bool success, const QString &message, const QVariantMap &data);
    void aiChatStreamChunk(const QString &sessionId, const QString &messageId, const QString &chunk);
    void aiChatFinished(bool success, const QString &message, const QVariantMap &data);

private:
    QJsonObject aiChatStateToJson() const;
    void loadAiChatState();
    void persistAiChatState();
    QString ensureAiChatSession(const QString &title = QString());
    int findAiChatSessionIndex(const QString &sessionId) const;
    QVariantMap aiChatSnapshot() const;
    static QJsonObject makeAiChatMessage(const QString &id, const QString &role, const QString &content, bool useDeviceContext);
    static QString trimForTitle(const QString &text);
    QString buildGeneralChatPrompt(const QString &sessionId, const QString &message, bool useDeviceContext,
                                   bool *deviceContextAttached) const;
    void pruneAiChatSessions();

    void updateSpectrumSeries();
    void updateTrendSeries();
    static double findPeak(const SpectrumFrame &frame, double mass, double tolerance);

    ApplicationContext context_;
    InstrumentStatus status_;
    SpectrumFrame latestFrame_;
    DeviceConnectionConfig connectionConfig_;

    QVector<QPointF> ricTrend_;
    QVector<QPointF> ticTrend_;
    int trendIndex_ = 0;
    const int maxTrendPoints_ = 240;
    double ticValue_ = 0.0;
    double ricValue_ = 0.0;
    double peak18_ = 0.0;
    double peak28_ = 0.0;
    double peak44_ = 0.0;

    QPointer<QXYSeries> spectrumSeries_;
    QPointer<QXYSeries> ricSeries_;
    QPointer<QXYSeries> ticSeries_;
    bool aiRunning_ = false;
    QString aiSummaryText_;
    QString aiTroubleshootText_;
    QString aiQuestion_;
    bool aiIncludeTroubleshoot_ = false;
    enum class AiTask {
        None,
        Summary,
        Troubleshoot,
        ExportSummary,
        ExportTroubleshoot,
        Chat
    };
    AiTask aiTask_ = AiTask::None;
    QJsonArray aiChatSessions_;
    QString aiChatActiveSessionId_;
    QString aiChatWorkingSessionId_;
    QString aiChatWorkingMessageId_;
    QString aiChatWorkingUserMessage_;
    QString aiChatWorkingAssistantText_;
    bool aiChatWorkingUseDeviceContext_ = false;
    bool aiChatWorkingDeviceContextAttached_ = false;
};

}  // namespace deviceapp
