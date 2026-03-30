#include "qml/app_state.h"

#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <QTimer>
#include <QUuid>
#include <QUrl>
#include <QVariantMap>
#include <QtMath>
#include <algorithm>

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

ScanSettings inferScanSettingsFromFrame(const SpectrumFrame &frame) {
    ScanSettings settings;
    settings.mode = frame.scanMode;
    if (!frame.masses.isEmpty()) {
        settings.massStart = frame.masses.first();
        settings.massEnd = frame.masses.last();
        if (settings.massEnd < settings.massStart) {
            std::swap(settings.massStart, settings.massEnd);
        }
    }
    return settings;
}

bool openExportReportAndFolder(const QString &reportPath, QString *error) {
    const QString path = reportPath.trimmed();
    if (path.isEmpty()) {
        if (error) {
            *error = QString::fromUtf8("报告路径为空");
        }
        return false;
    }

    QFileInfo reportInfo(path);
    QStringList failures;

    if (!reportInfo.exists()) {
        failures.push_back(QString::fromUtf8("报告文件不存在"));
    } else if (!QDesktopServices::openUrl(QUrl::fromLocalFile(reportInfo.absoluteFilePath()))) {
        failures.push_back(QString::fromUtf8("打开报告文件失败"));
    }

    const QString folderPath = reportInfo.absolutePath();
    if (folderPath.isEmpty() || !QDir(folderPath).exists()) {
        failures.push_back(QString::fromUtf8("报告目录不存在"));
    } else if (!QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath))) {
        failures.push_back(QString::fromUtf8("打开报告目录失败"));
    }

    if (!failures.isEmpty()) {
        if (error) {
            *error = failures.join(QString::fromUtf8("，"));
        }
        return false;
    }
    return true;
}

QJsonObject buildFrameSummary(const SpectrumFrame &frame) {
    double tic = 0.0;
    double maxIntensity = 0.0;
    double minMass = 0.0;
    double maxMass = 0.0;
    if (!frame.masses.isEmpty()) {
        minMass = frame.masses.first();
        maxMass = frame.masses.last();
    }
    for (double v : frame.intensities) {
        tic += v;
        maxIntensity = qMax(maxIntensity, v);
    }

    QJsonArray peaks;
    const int peakCount = qMin(5, frame.peaks.size());
    for (int i = 0; i < peakCount; ++i) {
        const PeakInfo &peak = frame.peaks[i];
        peaks.append(QJsonObject{{"mass", peak.mass}, {"intensity", peak.intensity}});
    }

    return QJsonObject{
        {"timestamp", frame.timestamp.toString(Qt::ISODate)},
        {"scanMode", scanModeToString(frame.scanMode)},
        {"pointCount", qMin(frame.masses.size(), frame.intensities.size())},
        {"massRange", QJsonArray{minMass, maxMass}},
        {"tic", tic},
        {"maxIntensity", maxIntensity},
        {"topPeaks", peaks},
    };
}

QJsonObject buildDeviceContextSummary(const InstrumentStatus &status, const SpectrumFrame &frame,
                                      const TuneParameters &tune, const ScanSettings &scan,
                                      double peak18, double peak28, double peak44) {
    return QJsonObject{
        {"status", QJsonObject{
            {"connected", status.connected},
            {"scanning", status.scanning},
            {"vacuumPa", status.vacuum.valuePa},
            {"lastError", status.lastError},
        }},
        {"scan", toJson(scan)},
        {"tune", toJson(tune)},
        {"frameSummary", buildFrameSummary(frame)},
        {"keyPeaks", QJsonObject{
            {"m18", peak18},
            {"m28", peak28},
            {"m44", peak44},
        }},
    };
}

QString buildSummaryPrompt(const InstrumentStatus &status, const SpectrumFrame &frame,
                           const TuneParameters &tune, const ScanSettings &scan) {
    const QJsonObject ctx{
        {"status", toJson(status)},
        {"frame", toJson(frame)},
        {"tune", toJson(tune)},
        {"scan", toJson(scan)},
    };
    return QString::fromUtf8(
               "你是四极杆质谱调谐助手。必须包含【思考过程】和【结论】两个部分，并严格按以下结构输出中文：\n"
               "【思考过程】\n"
               "- 先解释你如何解读关键数据（可分点）\n"
               "- 指出不确定性与假设\n"
               "【结论】\n"
               "1) 当前状态总结\n2) 关键异常/风险\n3) 建议参数调整（给出方向和幅度）\n4) 下一步操作。\n"
               "若数据不足要明确指出。\n\nJSON:\n") +
           QString::fromUtf8(QJsonDocument(ctx).toJson(QJsonDocument::Compact));
}

QString buildTroubleshootPrompt(const InstrumentStatus &status, const SpectrumFrame &frame,
                                const TuneParameters &tune, const ScanSettings &scan,
                                const QString &question) {
    const QJsonObject ctx{
        {"status", toJson(status)},
        {"frame", toJson(frame)},
        {"tune", toJson(tune)},
        {"scan", toJson(scan)},
    };
    return QString::fromUtf8("你是四极杆质谱故障诊断助手。用户问题：") + question +
           QString::fromUtf8(
               "\n必须包含【思考过程】和【结论】两个部分，并输出固定结构：\n"
               "【思考过程】\n"
               "- 对问题进行原因推理链路说明（分点）\n"
               "- 给出你排除其它原因的依据\n"
               "【结论】\n"
               "1) 可能原因（最多3条）\n2) 排查步骤（可执行）\n3) 参数建议（含预期影响）\n4) 风险提示。\n"
               "结合以下 JSON：\n") +
           QString::fromUtf8(QJsonDocument(ctx).toJson(QJsonDocument::Compact));
}

QString composeAiReport(const QString &summary, const QString &troubleshoot) {
    QString report;
    report += "调谐 AI 报告\n";
    report += "生成时间: " + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") + "\n\n";
    report += "[AI总结]\n";
    report += summary.trimmed().isEmpty() ? "无\n" : summary.trimmed() + "\n";
    report += "\n[疑难解答]\n";
    report += troubleshoot.trimmed().isEmpty() ? "无\n" : troubleshoot.trimmed() + "\n";
    return report;
}

QString nowIso8601() {
    return QDateTime::currentDateTime().toString(Qt::ISODate);
}

constexpr int kAiChatMaxSessions = 5;
constexpr int kAiChatPromptHistoryMessages = 10;
constexpr int kAiSummaryHistoryMaxItems = 50;

QJsonObject normalizeSummaryHistoryItem(const QJsonObject &raw) {
    QJsonObject item = raw;
    if (item.value("id").toString().trimmed().isEmpty()) {
        item["id"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    if (item.value("createdAt").toString().trimmed().isEmpty()) {
        item["createdAt"] = nowIso8601();
    }
    if (item.value("model").toString().trimmed().isEmpty()) {
        item["model"] = QString::fromUtf8("未设置");
    }
    item["content"] = item.value("content").toString();
    return item;
}

QVariantMap okResult(const QVariantMap &data) {
    return QVariantMap{{"ok", true}, {"data", data}};
}

QVariantMap errorResult(const QString &message) {
    return QVariantMap{{"ok", false}, {"error", message}};
}

}  // namespace

AppState::AppState(QObject *parent) : QObject(parent) {
    connectionConfig_ = context_.connectionService->recentConnection();
    loadAiChatState();
    aiProviderConfig();

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

    QObject::connect(context_.aiAssistantService.get(), &AiAssistantService::streamChunk, static_cast<QObject *>(this),
                     [this](const QString &chunk) {
        if (!aiRunning_ || chunk.isEmpty()) {
            return;
        }
        if (aiTask_ == AiTask::Chat) {
            aiChatWorkingAssistantText_.append(chunk);
            emit aiChatStreamChunk(aiChatWorkingSessionId_, aiChatWorkingMessageId_, chunk);
            return;
        }
        if (aiTask_ == AiTask::Summary || aiTask_ == AiTask::ExportSummary) {
            aiSummaryText_.append(chunk);
        } else if (aiTask_ == AiTask::Troubleshoot || aiTask_ == AiTask::ExportTroubleshoot) {
            aiTroubleshootText_.append(chunk);
        }
        emit aiStreamChunk(chunk);
    });

    QObject::connect(context_.aiAssistantService.get(), &AiAssistantService::streamCompleted, static_cast<QObject *>(this),
                     [this](const AiResult &result) {
        const AiTask finishedTask = aiTask_;
        const QString chatSessionId = aiChatWorkingSessionId_;
        const QString chatMessageId = aiChatWorkingMessageId_;

        auto finishChat = [this, &chatSessionId, &chatMessageId](bool success, const QString &message) {
            const int idx = findAiChatSessionIndex(chatSessionId);
            if (idx >= 0) {
                QJsonObject session = aiChatSessions_.at(idx).toObject();
                QJsonArray messages = session.value("messages").toArray();
                for (int i = 0; i < messages.size(); ++i) {
                    QJsonObject msg = messages.at(i).toObject();
                    if (msg.value("id").toString() == chatMessageId) {
                        if (success) {
                            msg["content"] = aiChatWorkingAssistantText_;
                        } else {
                            QString content = aiChatWorkingAssistantText_.trimmed();
                            if (content.isEmpty()) {
                                content = QString::fromUtf8("[生成失败] ") + message;
                            } else {
                                content += QString::fromUtf8("\n\n[生成失败] ") + message;
                            }
                            msg["content"] = content;
                        }
                        messages[i] = msg;
                        break;
                    }
                }
                session["messages"] = messages;
                session["updatedAt"] = nowIso8601();
                aiChatSessions_[idx] = session;
                persistAiChatState();
            }
            aiRunning_ = false;
            aiTask_ = AiTask::None;
            aiChatWorkingSessionId_.clear();
            aiChatWorkingMessageId_.clear();
            aiChatWorkingUserMessage_.clear();
            aiChatWorkingAssistantText_.clear();
            aiChatWorkingUseDeviceContext_ = false;
            aiChatWorkingDeviceContextAttached_ = false;
            emit aiRunningChanged();
            QVariantMap data = aiChatSnapshot();
            data.insert("sessionId", chatSessionId);
            data.insert("messageId", chatMessageId);
            emit aiChatFinished(success, message, data);
        };

        if (finishedTask == AiTask::Chat) {
            if (!result.success) {
                finishChat(false, result.message);
                emit aiRequestFinished(false, result.message, QVariantMap{{"type", "chat"}});
                return;
            }
            finishChat(true, QString());
            emit aiRequestFinished(true, QString(), QVariantMap{{"type", "chat"}});
            return;
        }

        if (!result.success) {
            aiRunning_ = false;
            aiTask_ = AiTask::None;
            emit aiRunningChanged();
            emit aiRequestFinished(false, result.message, QVariantMap());
            return;
        }

        auto finishExport = [this]() {
            const TuneParameters tune = context_.tuneService->currentParameters();
            const ScanSettings scan = inferScanSettingsFromFrame(latestFrame_);
            const QString frameJsonPath = context_.persistenceService->saveFrame(latestFrame_);
            const QString frameCsvPath = context_.persistenceService->exportFrameCsv(latestFrame_);
            const QJsonObject snapshot{
                {"status", toJson(status_)},
                {"scan", toJson(scan)},
                {"tune", toJson(tune)},
                {"frame", toJson(latestFrame_)},
            };
            const QString snapshotPath = context_.persistenceService->saveSnapshotJson(snapshot, "tune_snapshot");
            const QString reportPath =
                context_.persistenceService->saveReportText(composeAiReport(aiSummaryText_, aiTroubleshootText_), "ai_report");
            QString openError;
            if (!openExportReportAndFolder(reportPath, &openError)) {
                emit toastRequested(QString::fromUtf8("导出成功，但自动打开失败: %1 (%2)").arg(openError, reportPath));
            }

            QVariantMap paths;
            paths.insert("frameJson", frameJsonPath);
            paths.insert("frameCsv", frameCsvPath);
            paths.insert("snapshotJson", snapshotPath);
            paths.insert("reportTxt", reportPath);

            QVariantMap data;
            data.insert("type", "export");
            data.insert("summary", aiSummaryText_);
            data.insert("troubleshoot", aiTroubleshootText_);
            data.insert("paths", paths);
            aiRunning_ = false;
            aiTask_ = AiTask::None;
            emit aiRunningChanged();
            emit aiRequestFinished(true, QString(), data);
        };

        if (finishedTask == AiTask::Summary) {
            aiRunning_ = false;
            aiTask_ = AiTask::None;
            emit aiRunningChanged();
            emit aiRequestFinished(true, QString(), QVariantMap{{"type", "summary"}, {"summary", aiSummaryText_}});
            return;
        }

        if (finishedTask == AiTask::Troubleshoot) {
            aiRunning_ = false;
            aiTask_ = AiTask::None;
            emit aiRunningChanged();
            emit aiRequestFinished(true, QString(), QVariantMap{{"type", "troubleshoot"}, {"answer", aiTroubleshootText_}});
            return;
        }

        if (finishedTask == AiTask::ExportSummary) {
            if (aiIncludeTroubleshoot_ && !aiQuestion_.isEmpty()) {
                aiTask_ = AiTask::ExportTroubleshoot;
                emit aiStreamChunk("\n\n");
                const TuneParameters tune = context_.tuneService->currentParameters();
                const ScanSettings scan = inferScanSettingsFromFrame(latestFrame_);
                const QString prompt = buildTroubleshootPrompt(status_, latestFrame_, tune, scan, aiQuestion_);
                if (!context_.aiAssistantService->startGenerateStream(prompt)) {
                    aiRunning_ = false;
                    aiTask_ = AiTask::None;
                    emit aiRunningChanged();
                    emit aiRequestFinished(false, "启动疑难解答失败", QVariantMap());
                }
                return;
            }
            finishExport();
            return;
        }

        if (finishedTask == AiTask::ExportTroubleshoot) {
            finishExport();
            return;
        }
    });

    context_.deviceAdapter->readStatusSnapshot();

#ifdef Q_OS_WASM
    auto *autoStartTimer = new QTimer(this);
    autoStartTimer->setInterval(300);
    connect(autoStartTimer, &QTimer::timeout, this, [this, autoStartTimer]() {
        if (!status_.connected) {
            const QString host = connectionConfig_.host.trimmed().isEmpty() ? "127.0.0.1" : connectionConfig_.host.trimmed();
            const int port = qBound(1, static_cast<int>(connectionConfig_.port), 65535);
            connectToDevice(host, port);
            return;
        }
        if (!status_.scanning) {
            startScan(10.0, 110.0);
            return;
        }
        autoStartTimer->stop();
        autoStartTimer->deleteLater();
    });
    autoStartTimer->start();
#endif
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

QVariantList AppState::webChartPoints(const QString &chartName) const {
    QVariantList points;
    const QString key = chartName.trimmed().toLower();

    if (key == "spectrum") {
        const int count = qMin(latestFrame_.masses.size(), latestFrame_.intensities.size());
        points.reserve(count);
        for (int i = 0; i < count; ++i) {
            QVariantMap point;
            point.insert("x", latestFrame_.masses[i]);
            point.insert("y", latestFrame_.intensities[i]);
            points.append(point);
        }
        return points;
    }

    const QVector<QPointF> *trend = nullptr;
    if (key == "ric") {
        trend = &ricTrend_;
    } else if (key == "tic") {
        trend = &ticTrend_;
    }

    if (!trend) {
        return points;
    }

    points.reserve(trend->size());
    for (const QPointF &p : *trend) {
        QVariantMap point;
        point.insert("x", p.x());
        point.insert("y", p.y());
        points.append(point);
    }
    return points;
}

QVariantMap AppState::aiModelStatus() {
    const AiResult result = context_.aiAssistantService->modelStatus();
    if (!result.success) {
        return errorResult(result.message);
    }
    return okResult(result.payload.toVariantMap());
}

QVariantMap AppState::aiProviderConfig() {
    QJsonObject config = context_.settingsService->aiProviderConfig();
    if (config.isEmpty()) {
        config = context_.aiAssistantService->providerConfig();
    } else {
        QString error;
        if (!context_.aiAssistantService->applyProviderConfig(config, &error)) {
            Q_UNUSED(error);
        }
    }
    return okResult(config.toVariantMap());
}

QVariantMap AppState::aiSaveProviderConfig(const QVariantMap &config) {
    const QJsonObject obj = QJsonObject::fromVariantMap(config);
    QString error;
    if (!context_.aiAssistantService->applyProviderConfig(obj, &error)) {
        return errorResult(error.isEmpty() ? "保存AI配置失败" : error);
    }
    context_.settingsService->saveAiProviderConfig(context_.aiAssistantService->providerConfig());
    return okResult(context_.aiAssistantService->providerConfig().toVariantMap());
}

QVariantMap AppState::aiPullModel(const QString &model) {
    const AiResult result = context_.aiAssistantService->pullModel(model.trimmed());
    if (!result.success) {
        return errorResult(result.message);
    }
    QVariantMap data;
    data.insert("model", model.trimmed().isEmpty() ? context_.aiAssistantService->modelTag() : model.trimmed());
    data.insert("status", result.message);
    return okResult(data);
}

QVariantMap AppState::aiTuneSummary() {
    if (!latestFrame_.timestamp.isValid()) {
        return errorResult("no frame available");
    }
    const TuneParameters tune = context_.tuneService->currentParameters();
    const ScanSettings scan = inferScanSettingsFromFrame(latestFrame_);
    const AiResult result = context_.aiAssistantService->summarizeTune(status_, latestFrame_, tune, scan);
    if (!result.success) {
        return errorResult(result.message);
    }
    return okResult(QVariantMap{{"summary", result.content}});
}

QVariantMap AppState::aiTuneTroubleshoot(const QString &question) {
    if (!latestFrame_.timestamp.isValid()) {
        return errorResult("no frame available");
    }
    const QString q = question.trimmed();
    if (q.isEmpty()) {
        return errorResult("question is required");
    }
    const TuneParameters tune = context_.tuneService->currentParameters();
    const ScanSettings scan = inferScanSettingsFromFrame(latestFrame_);
    const AiResult result = context_.aiAssistantService->troubleshootTune(status_, latestFrame_, tune, scan, q);
    if (!result.success) {
        return errorResult(result.message);
    }
    return okResult(QVariantMap{{"answer", result.content}});
}

QVariantMap AppState::aiTuneExport(const QString &question, bool includeTroubleshoot) {
    if (!latestFrame_.timestamp.isValid()) {
        return errorResult("no frame available");
    }
    const QString q = question.trimmed();
    const TuneParameters tune = context_.tuneService->currentParameters();
    const ScanSettings scan = inferScanSettingsFromFrame(latestFrame_);

    const AiResult summary = context_.aiAssistantService->summarizeTune(status_, latestFrame_, tune, scan);
    if (!summary.success) {
        return errorResult(summary.message);
    }

    QString troubleshootText;
    if (includeTroubleshoot && !q.isEmpty()) {
        const AiResult troubleshoot = context_.aiAssistantService->troubleshootTune(status_, latestFrame_, tune, scan, q);
        if (!troubleshoot.success) {
            return errorResult(troubleshoot.message);
        }
        troubleshootText = troubleshoot.content;
    }

    const QString frameJsonPath = context_.persistenceService->saveFrame(latestFrame_);
    const QString frameCsvPath = context_.persistenceService->exportFrameCsv(latestFrame_);
    const QJsonObject snapshot{
        {"status", toJson(status_)},
        {"scan", toJson(scan)},
        {"tune", toJson(tune)},
        {"frame", toJson(latestFrame_)},
    };
    const QString snapshotPath = context_.persistenceService->saveSnapshotJson(snapshot, "tune_snapshot");
    const QString reportPath =
        context_.persistenceService->saveReportText(composeAiReport(summary.content, troubleshootText), "ai_report");
    QString openError;
    if (!openExportReportAndFolder(reportPath, &openError)) {
        emit toastRequested(QString::fromUtf8("导出成功，但自动打开失败: %1 (%2)").arg(openError, reportPath));
    }

    QVariantMap paths;
    paths.insert("frameJson", frameJsonPath);
    paths.insert("frameCsv", frameCsvPath);
    paths.insert("snapshotJson", snapshotPath);
    paths.insert("reportTxt", reportPath);

    QVariantMap data;
    data.insert("summary", summary.content);
    data.insert("troubleshoot", troubleshootText);
    data.insert("paths", paths);
    return okResult(data);
}

QVariantMap AppState::aiSummaryHistory() {
    const QJsonObject state = context_.settingsService->aiSummaryHistory();
    const QJsonArray rawItems = state.value("items").toArray();
    QJsonArray items;
    for (const QJsonValue &v : rawItems) {
        if (!v.isObject()) {
            continue;
        }
        const QJsonObject normalized = normalizeSummaryHistoryItem(v.toObject());
        if (normalized.value("content").toString().trimmed().isEmpty()) {
            continue;
        }
        items.append(normalized);
        if (items.size() >= kAiSummaryHistoryMaxItems) {
            break;
        }
    }
    return okResult(QVariantMap{{"items", items.toVariantList()}});
}

QVariantMap AppState::aiSummaryAppend(const QVariantMap &entry) {
    QJsonObject item = normalizeSummaryHistoryItem(QJsonObject::fromVariantMap(entry));
    if (item.value("content").toString().trimmed().isEmpty()) {
        return errorResult("summary content is required");
    }

    QJsonObject state = context_.settingsService->aiSummaryHistory();
    QJsonArray items = state.value("items").toArray();
    QJsonArray next;
    next.append(item);
    for (const QJsonValue &v : items) {
        if (!v.isObject()) {
            continue;
        }
        const QJsonObject old = normalizeSummaryHistoryItem(v.toObject());
        if (old.value("id").toString() == item.value("id").toString()) {
            continue;
        }
        if (old.value("content").toString().trimmed().isEmpty()) {
            continue;
        }
        next.append(old);
        if (next.size() >= kAiSummaryHistoryMaxItems) {
            break;
        }
    }
    state["items"] = next;
    context_.settingsService->saveAiSummaryHistory(state);
    return okResult(QVariantMap{
        {"item", item.toVariantMap()},
        {"items", next.toVariantList()},
    });
}

QVariantMap AppState::aiSummaryExportText(const QString &summary, const QString &model, const QString &createdAt) {
    const QString content = summary.trimmed();
    if (content.isEmpty()) {
        return errorResult("summary content is required");
    }
    QString report;
    report += "AI 总结报告\n";
    report += "导出时间: " + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") + "\n";
    report += "模型: " + (model.trimmed().isEmpty() ? QString::fromUtf8("未设置") : model.trimmed()) + "\n";
    if (!createdAt.trimmed().isEmpty()) {
        report += "总结时间: " + createdAt.trimmed() + "\n";
    }
    report += "\n[AI总结]\n";
    report += content + "\n";

    const QString reportPath = context_.persistenceService->saveReportText(report, "ai_report");
    QString openError;
    if (!openExportReportAndFolder(reportPath, &openError)) {
        emit toastRequested(QString::fromUtf8("导出成功，但自动打开失败: %1 (%2)").arg(openError, reportPath));
    }
    return okResult(QVariantMap{
        {"reportTxt", reportPath},
    });
}

bool AppState::aiTuneSummaryAsync() {
    if (aiRunning_) {
        return false;
    }
    if (!latestFrame_.timestamp.isValid()) {
        emit aiRequestFinished(false, "no frame available", QVariantMap());
        return false;
    }
    const TuneParameters tune = context_.tuneService->currentParameters();
    const ScanSettings scan = inferScanSettingsFromFrame(latestFrame_);
    const QString prompt = buildSummaryPrompt(status_, latestFrame_, tune, scan);

    aiSummaryText_.clear();
    aiTroubleshootText_.clear();
    aiQuestion_.clear();
    aiIncludeTroubleshoot_ = false;
    aiTask_ = AiTask::Summary;
    aiRunning_ = true;
    emit aiRunningChanged();
    if (!context_.aiAssistantService->startGenerateStream(prompt)) {
        aiRunning_ = false;
        aiTask_ = AiTask::None;
        emit aiRunningChanged();
        emit aiRequestFinished(false, "启动AI总结失败", QVariantMap());
        return false;
    }
    return true;
}

bool AppState::aiTuneTroubleshootAsync(const QString &question) {
    if (aiRunning_) {
        return false;
    }
    if (!latestFrame_.timestamp.isValid()) {
        emit aiRequestFinished(false, "no frame available", QVariantMap());
        return false;
    }
    const QString q = question.trimmed();
    if (q.isEmpty()) {
        emit aiRequestFinished(false, "question is required", QVariantMap());
        return false;
    }
    const TuneParameters tune = context_.tuneService->currentParameters();
    const ScanSettings scan = inferScanSettingsFromFrame(latestFrame_);
    const QString prompt = buildTroubleshootPrompt(status_, latestFrame_, tune, scan, q);

    aiSummaryText_.clear();
    aiTroubleshootText_.clear();
    aiQuestion_ = q;
    aiIncludeTroubleshoot_ = true;
    aiTask_ = AiTask::Troubleshoot;
    aiRunning_ = true;
    emit aiRunningChanged();
    if (!context_.aiAssistantService->startGenerateStream(prompt)) {
        aiRunning_ = false;
        aiTask_ = AiTask::None;
        emit aiRunningChanged();
        emit aiRequestFinished(false, "启动疑难解答失败", QVariantMap());
        return false;
    }
    return true;
}

bool AppState::aiTuneExportAsync(const QString &question, bool includeTroubleshoot) {
    if (aiRunning_) {
        return false;
    }
    if (!latestFrame_.timestamp.isValid()) {
        emit aiRequestFinished(false, "no frame available", QVariantMap());
        return false;
    }
    const TuneParameters tune = context_.tuneService->currentParameters();
    const ScanSettings scan = inferScanSettingsFromFrame(latestFrame_);
    const QString prompt = buildSummaryPrompt(status_, latestFrame_, tune, scan);

    aiSummaryText_.clear();
    aiTroubleshootText_.clear();
    aiQuestion_ = question.trimmed();
    aiIncludeTroubleshoot_ = includeTroubleshoot;
    aiTask_ = AiTask::ExportSummary;
    aiRunning_ = true;
    emit aiRunningChanged();
    if (!context_.aiAssistantService->startGenerateStream(prompt)) {
        aiRunning_ = false;
        aiTask_ = AiTask::None;
        emit aiRunningChanged();
        emit aiRequestFinished(false, "启动导出任务失败", QVariantMap());
        return false;
    }
    return true;
}

void AppState::aiCancel() {
    if (!aiRunning_) {
        return;
    }
    context_.aiAssistantService->cancelStream();
}

QJsonObject AppState::aiChatStateToJson() const {
    return QJsonObject{
        {"activeSessionId", aiChatActiveSessionId_},
        {"sessions", aiChatSessions_},
    };
}

void AppState::loadAiChatState() {
    const QJsonObject state = context_.settingsService->aiChatState();
    aiChatSessions_ = state.value("sessions").toArray();
    aiChatActiveSessionId_ = state.value("activeSessionId").toString().trimmed();
    pruneAiChatSessions();
    if (aiChatActiveSessionId_.isEmpty() || findAiChatSessionIndex(aiChatActiveSessionId_) < 0) {
        if (!aiChatSessions_.isEmpty()) {
            aiChatActiveSessionId_ = aiChatSessions_.first().toObject().value("id").toString();
        } else {
            aiChatActiveSessionId_ = ensureAiChatSession();
        }
    }
    persistAiChatState();
}

void AppState::persistAiChatState() {
    context_.settingsService->saveAiChatState(aiChatStateToJson());
}

QString AppState::ensureAiChatSession(const QString &title) {
    if (!aiChatActiveSessionId_.isEmpty() && findAiChatSessionIndex(aiChatActiveSessionId_) >= 0) {
        return aiChatActiveSessionId_;
    }
    QJsonObject session;
    const QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    session["id"] = id;
    session["title"] = title.trimmed().isEmpty() ? "新会话" : title.trimmed();
    session["updatedAt"] = nowIso8601();
    session["messages"] = QJsonArray{};
    aiChatSessions_.prepend(session);
    aiChatActiveSessionId_ = id;
    pruneAiChatSessions();
    persistAiChatState();
    return id;
}

int AppState::findAiChatSessionIndex(const QString &sessionId) const {
    if (sessionId.trimmed().isEmpty()) {
        return -1;
    }
    for (int i = 0; i < aiChatSessions_.size(); ++i) {
        if (aiChatSessions_.at(i).toObject().value("id").toString() == sessionId) {
            return i;
        }
    }
    return -1;
}

QVariantMap AppState::aiChatSnapshot() const {
    QVariantMap data;
    data.insert("activeSessionId", aiChatActiveSessionId_);
    data.insert("sessions", aiChatSessions_.toVariantList());
    return data;
}

QJsonObject AppState::makeAiChatMessage(const QString &id, const QString &role, const QString &content, bool useDeviceContext) {
    return QJsonObject{
        {"id", id},
        {"role", role},
        {"content", content},
        {"timestamp", nowIso8601()},
        {"useDeviceContext", useDeviceContext},
    };
}

QString AppState::trimForTitle(const QString &text) {
    const QString compact = text.simplified();
    if (compact.size() <= 20) {
        return compact;
    }
    return compact.left(20);
}

QString AppState::buildGeneralChatPrompt(const QString &sessionId, const QString &message, bool useDeviceContext,
                                         bool *deviceContextAttached) const {
    if (deviceContextAttached) {
        *deviceContextAttached = false;
    }
    QString prompt = QString::fromUtf8(
        "你是四极杆质谱助手。无论用户问题是否与设备相关，都用中文回答。\n"
        "必须按固定结构输出：\n"
        "【思考过程】\n"
        "- 给出推理步骤（分点）\n"
        "- 明确不确定性和假设\n"
        "【结论】\n"
        "1) 结论\n2) 可执行建议\n3) 注意事项。\n");

    const int idx = findAiChatSessionIndex(sessionId);
    if (idx >= 0) {
        const QJsonArray messages = aiChatSessions_.at(idx).toObject().value("messages").toArray();
        const int begin = qMax(0, messages.size() - kAiChatPromptHistoryMessages);
        prompt += "\n历史对话（最近消息）:\n";
        for (int i = begin; i < messages.size(); ++i) {
            const QJsonObject msg = messages.at(i).toObject();
            const QString role = msg.value("role").toString() == "assistant" ? "助手" : "用户";
            const QString content = msg.value("content").toString();
            if (content.trimmed().isEmpty()) {
                continue;
            }
            prompt += role + ": " + content + "\n";
        }
    }

    if (useDeviceContext && latestFrame_.timestamp.isValid()) {
        const TuneParameters tune = context_.tuneService->currentParameters();
        const ScanSettings scan = inferScanSettingsFromFrame(latestFrame_);
        const QJsonObject ctx = buildDeviceContextSummary(status_, latestFrame_, tune, scan, peak18_, peak28_, peak44_);
        prompt += "\n设备实时上下文摘要(JSON):\n";
        prompt += QString::fromUtf8(QJsonDocument(ctx).toJson(QJsonDocument::Compact));
        prompt += "\n";
        if (deviceContextAttached) {
            *deviceContextAttached = true;
        }
    }

    prompt += "\n当前用户问题:\n" + message.trimmed() + "\n";
    return prompt;
}

void AppState::pruneAiChatSessions() {
    while (aiChatSessions_.size() > kAiChatMaxSessions) {
        aiChatSessions_.removeLast();
    }
}

QVariantMap AppState::aiChatBootstrap() {
    ensureAiChatSession();
    return okResult(aiChatSnapshot());
}

QVariantMap AppState::aiChatNewSession() {
    const QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QJsonObject session;
    session["id"] = id;
    session["title"] = "新会话";
    session["updatedAt"] = nowIso8601();
    session["messages"] = QJsonArray{};
    aiChatSessions_.prepend(session);
    aiChatActiveSessionId_ = id;
    pruneAiChatSessions();
    persistAiChatState();
    QVariantMap data = aiChatSnapshot();
    data.insert("sessionId", id);
    return okResult(data);
}

QVariantMap AppState::aiChatSwitchSession(const QString &sessionId) {
    const QString id = sessionId.trimmed();
    const int idx = findAiChatSessionIndex(id);
    if (idx < 0) {
        return errorResult("session not found");
    }
    aiChatActiveSessionId_ = id;
    QJsonObject session = aiChatSessions_.at(idx).toObject();
    aiChatSessions_.removeAt(idx);
    aiChatSessions_.prepend(session);
    persistAiChatState();
    return okResult(aiChatSnapshot());
}

QVariantMap AppState::aiChatSendAsync(const QString &message, bool useDeviceContext) {
    if (aiRunning_) {
        return errorResult("ai is busy");
    }
    const QString q = message.trimmed();
    if (q.isEmpty()) {
        return errorResult("question is required");
    }
    const QString sessionId = ensureAiChatSession();
    const int idx = findAiChatSessionIndex(sessionId);
    if (idx < 0) {
        return errorResult("session not found");
    }

    QJsonObject session = aiChatSessions_.at(idx).toObject();
    QJsonArray messages = session.value("messages").toArray();
    const QString userId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const QString assistantId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    messages.append(makeAiChatMessage(userId, "user", q, useDeviceContext));
    messages.append(makeAiChatMessage(assistantId, "assistant", "", useDeviceContext));
    session["messages"] = messages;
    if (session.value("title").toString().trimmed().isEmpty() || session.value("title").toString() == "新会话") {
        session["title"] = trimForTitle(q);
    }
    session["updatedAt"] = nowIso8601();
    aiChatSessions_.removeAt(idx);
    aiChatSessions_.prepend(session);
    aiChatActiveSessionId_ = sessionId;
    pruneAiChatSessions();
    persistAiChatState();

    bool contextAttached = false;
    const QString prompt = buildGeneralChatPrompt(sessionId, q, useDeviceContext, &contextAttached);

    aiRunning_ = true;
    aiTask_ = AiTask::Chat;
    aiChatWorkingSessionId_ = sessionId;
    aiChatWorkingMessageId_ = assistantId;
    aiChatWorkingUserMessage_ = q;
    aiChatWorkingAssistantText_.clear();
    aiChatWorkingUseDeviceContext_ = useDeviceContext;
    aiChatWorkingDeviceContextAttached_ = contextAttached;
    emit aiRunningChanged();

    if (!context_.aiAssistantService->startGenerateStream(prompt)) {
        aiRunning_ = false;
        aiTask_ = AiTask::None;
        aiChatWorkingSessionId_.clear();
        aiChatWorkingMessageId_.clear();
        aiChatWorkingUserMessage_.clear();
        aiChatWorkingAssistantText_.clear();
        aiChatWorkingUseDeviceContext_ = false;
        aiChatWorkingDeviceContextAttached_ = false;
        emit aiRunningChanged();
        return errorResult("启动聊天失败");
    }

    QVariantMap data = aiChatSnapshot();
    data.insert("sessionId", sessionId);
    data.insert("userMessageId", userId);
    data.insert("assistantMessageId", assistantId);
    data.insert("deviceContextAttached", contextAttached);
    return okResult(data);
}

QVariantMap AppState::aiChatClearCurrentSession() {
    const int idx = findAiChatSessionIndex(aiChatActiveSessionId_);
    if (idx < 0) {
        return errorResult("session not found");
    }
    QJsonObject session = aiChatSessions_.at(idx).toObject();
    session["messages"] = QJsonArray{};
    session["updatedAt"] = nowIso8601();
    aiChatSessions_[idx] = session;
    persistAiChatState();
    return okResult(aiChatSnapshot());
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
