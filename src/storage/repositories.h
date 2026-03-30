#pragma once

#include <QObject>
#include <QJsonObject>

#include "core/types.h"

namespace deviceapp {

class SettingsRepository : public QObject {
    Q_OBJECT

public:
    explicit SettingsRepository(QObject *parent = nullptr);

    DeviceConnectionConfig loadRecentConnection() const;
    void saveRecentConnection(const DeviceConnectionConfig &config) const;
    TuneParameters loadTuneParameters() const;
    void saveTuneParameters(const TuneParameters &parameters) const;
    DataProcessingSettings loadDataProcessingSettings() const;
    void saveDataProcessingSettings(const DataProcessingSettings &settings) const;
    SystemSettings loadSystemSettings() const;
    void saveSystemSettings(const SystemSettings &settings) const;
    ChartSettings loadChartSettings() const;
    void saveChartSettings(const ChartSettings &settings) const;
    InstrumentControlSettings loadInstrumentControlSettings() const;
    void saveInstrumentControlSettings(const InstrumentControlSettings &settings) const;
    QJsonObject loadAiChatState() const;
    void saveAiChatState(const QJsonObject &state) const;
    QJsonObject loadAiProviderConfig() const;
    void saveAiProviderConfig(const QJsonObject &config) const;
    QJsonObject loadAiSummaryHistory() const;
    void saveAiSummaryHistory(const QJsonObject &history) const;
};

class MethodRepository : public QObject {
    Q_OBJECT

public:
    explicit MethodRepository(QObject *parent = nullptr);

    QString saveMonitorMethod(const MonitorMethod &method) const;
    MonitorMethod loadMonitorMethod(const QString &filePath) const;
    QStringList listMonitorMethods() const;
};

class FrameRepository : public QObject {
    Q_OBJECT

public:
    explicit FrameRepository(QObject *parent = nullptr);

    QString saveFrame(const SpectrumFrame &frame) const;
    SpectrumFrame loadFrame(const QString &filePath) const;
    QString exportFrameCsv(const SpectrumFrame &frame, const QString &baseName = QString()) const;
    QString saveJsonBlob(const QJsonObject &object, const QString &baseName = QString()) const;
    QString saveTextBlob(const QString &text, const QString &baseName = QString(),
                         const QString &suffix = ".txt") const;
};

}  // namespace deviceapp
