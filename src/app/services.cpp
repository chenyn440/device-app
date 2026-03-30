#include "app/services.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

namespace deviceapp {

ConnectionService::ConnectionService(IDeviceAdapter *deviceAdapter, SettingsRepository *settingsRepository, QObject *parent)
    : QObject(parent), deviceAdapter_(deviceAdapter), settingsRepository_(settingsRepository) {}

DeviceConnectionConfig ConnectionService::recentConnection() const {
    return settingsRepository_->loadRecentConnection();
}

void ConnectionService::connectToDevice(const DeviceConnectionConfig &config) {
    settingsRepository_->saveRecentConnection(config);
    emit recentConnectionChanged(config);
    deviceAdapter_->connectToDevice(config);
}

void ConnectionService::disconnectFromDevice() {
    deviceAdapter_->disconnectFromDevice();
}

ScanControlService::ScanControlService(IDeviceAdapter *deviceAdapter, QObject *parent)
    : QObject(parent), deviceAdapter_(deviceAdapter) {}

void ScanControlService::updateInstrumentStatus(const InstrumentStatus &status) {
    currentStatus_ = status;
}

bool ScanControlService::canStartScan(const ScanSettings &settings, QString *errorMessage) const {
    if (!currentStatus_.connected) {
        if (errorMessage) {
            *errorMessage = "设备未连接";
        }
        return false;
    }
    if (!currentStatus_.vacuum.isReady()) {
        if (errorMessage) {
            *errorMessage = "当前真空度不满足扫描前置条件";
        }
        return false;
    }
    return settings.isValid(errorMessage);
}

void ScanControlService::startScan(const ScanSettings &settings) {
    QString errorMessage;
    if (!canStartScan(settings, &errorMessage)) {
        emit validationFailed(errorMessage);
        return;
    }
    deviceAdapter_->applyScanSettings(settings);
    deviceAdapter_->startScan(settings.mode);
}

void ScanControlService::stopScan() {
    deviceAdapter_->stopScan();
}

void ScanControlService::calibrateMassAxis() {
    deviceAdapter_->calibrateMassAxis();
}

TuneService::TuneService(IDeviceAdapter *deviceAdapter, SettingsRepository *settingsRepository, QObject *parent)
    : QObject(parent),
      deviceAdapter_(deviceAdapter),
      settingsRepository_(settingsRepository),
      currentParameters_(settingsRepository->loadTuneParameters()) {}

TuneParameters TuneService::currentParameters() const {
    return currentParameters_;
}

void TuneService::applyTuneParameters(const TuneParameters &parameters) {
    currentParameters_ = parameters;
    settingsRepository_->saveTuneParameters(parameters);
    deviceAdapter_->applyTuneParameters(parameters);
}

MonitorService::MonitorService(IDeviceAdapter *deviceAdapter, MethodRepository *methodRepository, QObject *parent)
    : QObject(parent), deviceAdapter_(deviceAdapter), methodRepository_(methodRepository) {}

QString MonitorService::saveMethod(const MonitorMethod &method) {
    return methodRepository_->saveMonitorMethod(method);
}

MonitorMethod MonitorService::loadMethod(const QString &path) const {
    return methodRepository_->loadMonitorMethod(path);
}

QStringList MonitorService::listMethods() const {
    return methodRepository_->listMonitorMethods();
}

void MonitorService::applyMethod(const MonitorMethod &method) {
    deviceAdapter_->applyScanSettings(method.scanSettings);
}

PersistenceService::PersistenceService(FrameRepository *frameRepository, QObject *parent)
    : QObject(parent), frameRepository_(frameRepository) {}

QString PersistenceService::saveFrame(const SpectrumFrame &frame) const {
    return frameRepository_->saveFrame(frame);
}

QString PersistenceService::exportFrameCsv(const SpectrumFrame &frame) const {
    return frameRepository_->exportFrameCsv(frame);
}

SpectrumFrame PersistenceService::loadFrame(const QString &path) const {
    return frameRepository_->loadFrame(path);
}

QString PersistenceService::saveSnapshotJson(const QJsonObject &snapshot, const QString &baseName) const {
    return frameRepository_->saveJsonBlob(snapshot, baseName);
}

QString PersistenceService::saveReportText(const QString &report, const QString &baseName) const {
    return frameRepository_->saveTextBlob(report, baseName, ".txt");
}

AiAssistantService::AiAssistantService(QObject *parent) : QObject(parent) {
    provider_ = qEnvironmentVariable("DEVICE_APP_AI_PROVIDER", "zhipu").trimmed().toLower();
    if (provider_.isEmpty()) {
        provider_ = "zhipu";
    }
    baseUrl_ = qEnvironmentVariable("DEVICE_APP_OLLAMA_URL", "http://127.0.0.1:11434").trimmed();
    if (baseUrl_.isEmpty()) {
        baseUrl_ = "http://127.0.0.1:11434";
    }
    if (provider_ == "zhipu" || provider_ == "cloud_zhipu") {
        zhipuBaseUrl_ =
            qEnvironmentVariable("DEVICE_APP_ZHIPU_BASE_URL", "https://open.bigmodel.cn/api/paas/v4").trimmed();
        zhipuApiKey_ = qEnvironmentVariable("DEVICE_APP_ZHIPU_API_KEY").trimmed();
        modelTag_ = qEnvironmentVariable("DEVICE_APP_ZHIPU_MODEL", "glm-4-flash").trimmed();
    } else if (provider_ == "gpt" || provider_ == "openai" || provider_ == "cloud_gpt") {
        zhipuBaseUrl_ = qEnvironmentVariable("DEVICE_APP_GPT_BASE_URL", "https://api.openai.com/v1").trimmed();
        zhipuApiKey_ = qEnvironmentVariable("DEVICE_APP_GPT_API_KEY").trimmed();
        modelTag_ = qEnvironmentVariable("DEVICE_APP_GPT_MODEL", "gpt-4o-mini").trimmed();
    } else if (provider_ == "qwen" || provider_ == "cloud_qwen") {
        zhipuBaseUrl_ =
            qEnvironmentVariable("DEVICE_APP_QWEN_BASE_URL", "https://dashscope.aliyuncs.com/compatible-mode/v1").trimmed();
        zhipuApiKey_ = qEnvironmentVariable("DEVICE_APP_QWEN_API_KEY").trimmed();
        modelTag_ = qEnvironmentVariable("DEVICE_APP_QWEN_MODEL", "qwen-max").trimmed();
    } else if (provider_ == "claude" || provider_ == "cloud_claude") {
        zhipuBaseUrl_ = qEnvironmentVariable("DEVICE_APP_CLAUDE_BASE_URL", "https://api.anthropic.com/v1").trimmed();
        zhipuApiKey_ = qEnvironmentVariable("DEVICE_APP_CLAUDE_API_KEY").trimmed();
        modelTag_ = qEnvironmentVariable("DEVICE_APP_CLAUDE_MODEL", "claude-3-5-sonnet-latest").trimmed();
    } else if (provider_ == "local_ollama") {
        zhipuBaseUrl_.clear();
        zhipuApiKey_.clear();
        modelTag_ = qEnvironmentVariable("DEVICE_APP_OLLAMA_MODEL", "qwen3.5:2b").trimmed();
    }
    if (zhipuBaseUrl_.isEmpty() && provider_ != "local_ollama") {
        zhipuBaseUrl_ = "https://open.bigmodel.cn/api/paas/v4";
    }
    if (zhipuApiKey_.isEmpty() && provider_ != "local_ollama") {
        zhipuApiKey_ = qEnvironmentVariable("DEVICE_APP_ZHIPU_API_KEY").trimmed();
    }
    if (modelTag_.isEmpty()) {
        modelTag_ = qEnvironmentVariable("DEVICE_APP_OLLAMA_MODEL", "glm-4-flash").trimmed();
    }
    if (modelTag_.isEmpty()) {
        modelTag_ = "glm-4-flash";
    }
    int configuredTimeout = qEnvironmentVariableIntValue("DEVICE_APP_AI_TIMEOUT_MS");
    if (configuredTimeout <= 0) {
        configuredTimeout = qEnvironmentVariableIntValue("DEVICE_APP_OLLAMA_TIMEOUT_MS");
    }
    timeoutMs_ = configuredTimeout > 0 ? qMax(3000, configuredTimeout) : 120000;
    streamTimeout_.setSingleShot(true);
    connect(&streamTimeout_, &QTimer::timeout, this, [this]() {
        if (!streamRunning_) {
            return;
        }
        if (streamReply_ && streamReply_->isRunning()) {
            streamReply_->abort();
        }
        AiResult result;
        result.success = false;
        result.message = "AI响应超时";
        finalizeStreamResult(result);
    });
}

bool AiAssistantService::useCloudProvider() const {
    return provider_ != "local_ollama";
}

bool AiAssistantService::useClaudeProvider() const {
    return provider_ == "claude" || provider_ == "cloud_claude";
}

QJsonObject AiAssistantService::providerConfig() const {
    return QJsonObject{
        {"provider", provider_},
        {"model", modelTag_},
        {"baseUrl", useClaudeProvider() ? zhipuBaseUrl_ : zhipuBaseUrl_},
        {"apiKey", zhipuApiKey_},
    };
}

bool AiAssistantService::applyProviderConfig(const QJsonObject &config, QString *errorMessage) {
    const QString provider = config.value("provider").toString().trimmed().toLower();
    const QString model = config.value("model").toString().trimmed();
    const QString baseUrl = config.value("baseUrl").toString().trimmed();
    const QString apiKey = config.value("apiKey").toString().trimmed();
    if (provider.isEmpty()) {
        if (errorMessage) *errorMessage = "provider is required";
        return false;
    }
    if (provider != "local_ollama" && apiKey.isEmpty()) {
        if (errorMessage) *errorMessage = "apiKey is required";
        return false;
    }
    provider_ = provider;
    if (!model.isEmpty()) {
        modelTag_ = model;
    }
    if (!baseUrl.isEmpty()) {
        zhipuBaseUrl_ = baseUrl;
    } else if (provider_ == "zhipu" || provider_ == "cloud_zhipu") {
        zhipuBaseUrl_ = "https://open.bigmodel.cn/api/paas/v4";
    } else if (provider_ == "gpt" || provider_ == "openai" || provider_ == "cloud_gpt") {
        zhipuBaseUrl_ = "https://api.openai.com/v1";
    } else if (provider_ == "qwen" || provider_ == "cloud_qwen") {
        zhipuBaseUrl_ = "https://dashscope.aliyuncs.com/compatible-mode/v1";
    } else if (provider_ == "claude" || provider_ == "cloud_claude") {
        zhipuBaseUrl_ = "https://api.anthropic.com/v1";
    }
    if (!apiKey.isEmpty()) {
        zhipuApiKey_ = apiKey;
    }
    return true;
}

QString AiAssistantService::resolveModelTag(const QString &modelTag) const {
    return modelTag.trimmed().isEmpty() ? modelTag_ : modelTag.trimmed();
}

AiResult AiAssistantService::requestJson(const QUrl &url, const QJsonObject &payload, const QByteArray &method) {
    AiResult result;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = nullptr;
    const QByteArray body = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    if (method == "GET") {
        reply = network_.get(request);
    } else {
        reply = network_.sendCustomRequest(request, method, body);
    }

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, [&]() {
        if (reply && reply->isRunning()) {
            reply->abort();
        }
        loop.quit();
    });
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start(timeoutMs_);
    loop.exec();

    if (!reply) {
        result.message = "网络请求创建失败";
        return result;
    }

    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray responseBytes = reply->readAll();
    const QString networkError = reply->error() == QNetworkReply::NoError ? QString() : reply->errorString();
    reply->deleteLater();

    if (!networkError.isEmpty()) {
        result.message = networkError;
        return result;
    }
    if (statusCode >= 400 || statusCode == 0) {
        result.message = QString("HTTP %1").arg(statusCode);
        return result;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(responseBytes);
    if (!doc.isObject()) {
        result.message = "返回不是 JSON 对象";
        return result;
    }
    result.success = true;
    result.payload = doc.object();
    return result;
}

QJsonObject AiAssistantService::buildTuneContext(const InstrumentStatus &status, const SpectrumFrame &frame,
                                                 const TuneParameters &tune, const ScanSettings &scan) const {
    return QJsonObject{
        {"status", toJson(status)},
        {"frame", toJson(frame)},
        {"tune", toJson(tune)},
        {"scan", toJson(scan)},
    };
}

QString AiAssistantService::extractZhipuContent(const QJsonObject &obj) {
    const QJsonArray choices = obj.value("choices").toArray();
    if (choices.isEmpty()) {
        return QString();
    }
    const QJsonObject first = choices.first().toObject();
    const QJsonObject delta = first.value("delta").toObject();
    const QString deltaContent = delta.value("content").toString();
    if (!deltaContent.isEmpty()) {
        return deltaContent;
    }
    return first.value("message").toObject().value("content").toString();
}

QString AiAssistantService::extractZhipuError(const QJsonObject &obj) {
    const QJsonValue err = obj.value("error");
    if (err.isString()) {
        return err.toString();
    }
    if (err.isObject()) {
        const QJsonObject eo = err.toObject();
        const QString msg = eo.value("message").toString();
        if (!msg.isEmpty()) {
            return msg;
        }
        const QString code = eo.value("code").toString();
        if (!code.isEmpty()) {
            return code;
        }
    }
    return QString();
}

QString AiAssistantService::extractClaudeContent(const QJsonObject &obj) {
    const QString type = obj.value("type").toString();
    if (type == "content_block_delta") {
        return obj.value("delta").toObject().value("text").toString();
    }
    const QJsonArray content = obj.value("content").toArray();
    if (!content.isEmpty()) {
        const QJsonObject first = content.first().toObject();
        return first.value("text").toString();
    }
    return QString();
}

QString AiAssistantService::extractClaudeError(const QJsonObject &obj) {
    if (obj.value("type").toString() == "error") {
        const QJsonObject error = obj.value("error").toObject();
        const QString message = error.value("message").toString();
        if (!message.isEmpty()) {
            return message;
        }
        return error.value("type").toString();
    }
    return QString();
}

AiResult AiAssistantService::requestZhipuGenerate(const QString &prompt, const QString &modelTag) {
    AiResult result;
    if (zhipuApiKey_.isEmpty()) {
        result.message = "智谱 API Key 未配置";
        return result;
    }

    const QJsonObject payload{
        {"model", resolveModelTag(modelTag)},
        {"messages", QJsonArray{QJsonObject{{"role", "user"}, {"content", prompt}}}},
        {"stream", false},
        {"temperature", 0.3},
    };

    QNetworkRequest request(QUrl(zhipuBaseUrl_ + "/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + zhipuApiKey_).toUtf8());
    QNetworkReply *reply =
        network_.sendCustomRequest(request, "POST", QJsonDocument(payload).toJson(QJsonDocument::Compact));
    if (!reply) {
        result.message = "网络请求创建失败";
        return result;
    }

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, [&]() {
        if (reply && reply->isRunning()) {
            reply->abort();
        }
        loop.quit();
    });
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start(timeoutMs_);
    loop.exec();

    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray responseBytes = reply->readAll();
    const QString networkError = reply->error() == QNetworkReply::NoError ? QString() : reply->errorString();
    reply->deleteLater();

    if (!networkError.isEmpty()) {
        result.message = networkError;
        return result;
    }
    if (statusCode >= 400 || statusCode == 0) {
        result.message = QString("HTTP %1").arg(statusCode);
        return result;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(responseBytes);
    if (!doc.isObject()) {
        result.message = "返回不是 JSON 对象";
        return result;
    }
    result.payload = doc.object();
    const QString err = extractZhipuError(result.payload);
    if (!err.isEmpty()) {
        result.message = err;
        return result;
    }
    result.content = extractZhipuContent(result.payload);
    if (result.content.trimmed().isEmpty()) {
        result.message = "模型未返回内容";
        return result;
    }
    result.success = true;
    return result;
}

AiResult AiAssistantService::requestClaudeGenerate(const QString &prompt, const QString &modelTag) {
    AiResult result;
    if (zhipuApiKey_.isEmpty()) {
        result.message = "Claude API Key 未配置";
        return result;
    }
    const QJsonObject payload{
        {"model", resolveModelTag(modelTag)},
        {"max_tokens", 1024},
        {"messages", QJsonArray{QJsonObject{{"role", "user"}, {"content", prompt}}}},
        {"stream", false},
    };
    QNetworkRequest request(QUrl(zhipuBaseUrl_ + "/messages"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("x-api-key", zhipuApiKey_.toUtf8());
    request.setRawHeader("anthropic-version", "2023-06-01");
    QNetworkReply *reply =
        network_.sendCustomRequest(request, "POST", QJsonDocument(payload).toJson(QJsonDocument::Compact));
    if (!reply) {
        result.message = "网络请求创建失败";
        return result;
    }
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, [&]() {
        if (reply && reply->isRunning()) {
            reply->abort();
        }
        loop.quit();
    });
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start(timeoutMs_);
    loop.exec();

    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray responseBytes = reply->readAll();
    const QString networkError = reply->error() == QNetworkReply::NoError ? QString() : reply->errorString();
    reply->deleteLater();
    if (!networkError.isEmpty()) {
        result.message = networkError;
        return result;
    }
    if (statusCode >= 400 || statusCode == 0) {
        result.message = QString("HTTP %1").arg(statusCode);
        return result;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(responseBytes);
    if (!doc.isObject()) {
        result.message = "返回不是 JSON 对象";
        return result;
    }
    result.payload = doc.object();
    const QString err = extractClaudeError(result.payload);
    if (!err.isEmpty()) {
        result.message = err;
        return result;
    }
    result.content = extractClaudeContent(result.payload);
    if (result.content.trimmed().isEmpty()) {
        result.message = "模型未返回内容";
        return result;
    }
    result.success = true;
    return result;
}

bool AiAssistantService::startZhipuGenerateStream(const QString &prompt, const QString &modelTag) {
    if (zhipuApiKey_.isEmpty()) {
        return false;
    }
    const QJsonObject payload{
        {"model", resolveModelTag(modelTag)},
        {"messages", QJsonArray{QJsonObject{{"role", "user"}, {"content", prompt}}}},
        {"stream", true},
        {"temperature", 0.3},
    };

    QNetworkRequest request(QUrl(zhipuBaseUrl_ + "/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + zhipuApiKey_).toUtf8());
    request.setRawHeader("Accept", "text/event-stream");
    QNetworkReply *reply =
        network_.sendCustomRequest(request, "POST", QJsonDocument(payload).toJson(QJsonDocument::Compact));
    if (!reply) {
        return false;
    }

    streamReply_ = reply;
    streamBuffer_.clear();
    streamAccumulated_.clear();
    streamCanceled_ = false;
    streamRunning_ = true;
    streamFormat_ = StreamFormat::Sse;
    streamTimeout_.start(timeoutMs_);

    connect(reply, &QNetworkReply::readyRead, this, &AiAssistantService::handleStreamReadyRead);
    connect(reply, &QNetworkReply::finished, this, [this]() {
        if (!streamReply_) {
            return;
        }
        handleStreamReadyRead();

        AiResult result;
        if (streamCanceled_) {
            result.success = false;
            result.message = "已取消生成";
            finalizeStreamResult(result);
            return;
        }
        const QString networkError = streamReply_->error() == QNetworkReply::NoError ? QString() : streamReply_->errorString();
        if (!networkError.isEmpty()) {
            result.success = false;
            result.message = networkError;
            finalizeStreamResult(result);
            return;
        }
        result.success = true;
        result.content = streamAccumulated_;
        if (result.content.trimmed().isEmpty()) {
            result.success = false;
            result.message = "模型未返回内容";
        }
        finalizeStreamResult(result);
    });
    return true;
}

bool AiAssistantService::startClaudeGenerateStream(const QString &prompt, const QString &modelTag) {
    if (zhipuApiKey_.isEmpty()) {
        return false;
    }
    const QJsonObject payload{
        {"model", resolveModelTag(modelTag)},
        {"max_tokens", 1024},
        {"messages", QJsonArray{QJsonObject{{"role", "user"}, {"content", prompt}}}},
        {"stream", true},
    };

    QNetworkRequest request(QUrl(zhipuBaseUrl_ + "/messages"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("x-api-key", zhipuApiKey_.toUtf8());
    request.setRawHeader("anthropic-version", "2023-06-01");
    request.setRawHeader("Accept", "text/event-stream");
    QNetworkReply *reply =
        network_.sendCustomRequest(request, "POST", QJsonDocument(payload).toJson(QJsonDocument::Compact));
    if (!reply) {
        return false;
    }

    streamReply_ = reply;
    streamBuffer_.clear();
    streamAccumulated_.clear();
    streamCanceled_ = false;
    streamRunning_ = true;
    streamFormat_ = StreamFormat::Sse;
    streamTimeout_.start(timeoutMs_);

    connect(reply, &QNetworkReply::readyRead, this, &AiAssistantService::handleStreamReadyRead);
    connect(reply, &QNetworkReply::finished, this, [this]() {
        if (!streamReply_) {
            return;
        }
        handleStreamReadyRead();

        AiResult result;
        if (streamCanceled_) {
            result.success = false;
            result.message = "已取消生成";
            finalizeStreamResult(result);
            return;
        }
        const QString networkError = streamReply_->error() == QNetworkReply::NoError ? QString() : streamReply_->errorString();
        if (!networkError.isEmpty()) {
            result.success = false;
            result.message = networkError;
            finalizeStreamResult(result);
            return;
        }
        result.success = true;
        result.content = streamAccumulated_;
        if (result.content.trimmed().isEmpty()) {
            result.success = false;
            result.message = "模型未返回内容";
        }
        finalizeStreamResult(result);
    });
    return true;
}

AiResult AiAssistantService::requestGenerate(const QString &prompt, const QString &modelTag) {
    if (useCloudProvider()) {
        if (useClaudeProvider()) {
            return requestClaudeGenerate(prompt, modelTag);
        }
        return requestZhipuGenerate(prompt, modelTag);
    }

    const QJsonObject payload{
        {"model", resolveModelTag(modelTag)},
        {"prompt", prompt},
        {"stream", false},
    };
    AiResult result = requestJson(QUrl(baseUrl_ + "/api/generate"), payload, "POST");
    if (!result.success) {
        return result;
    }
    const QString error = result.payload.value("error").toString();
    if (!error.isEmpty()) {
        result.success = false;
        result.message = error;
        return result;
    }
    result.content = result.payload.value("response").toString();
    if (result.content.trimmed().isEmpty()) {
        result.success = false;
        result.message = "模型未返回内容";
    }
    return result;
}

bool AiAssistantService::startGenerateStream(const QString &prompt, const QString &modelTag) {
    if (streamRunning_) {
        return false;
    }
    if (useCloudProvider()) {
        if (useClaudeProvider()) {
            return startClaudeGenerateStream(prompt, modelTag);
        }
        return startZhipuGenerateStream(prompt, modelTag);
    }

    const QString resolvedModel = resolveModelTag(modelTag);
    const QJsonObject payload{
        {"model", resolvedModel},
        {"prompt", prompt},
        {"stream", true},
    };

    QNetworkRequest request(QUrl(baseUrl_ + "/api/generate"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = network_.sendCustomRequest(request, "POST", QJsonDocument(payload).toJson(QJsonDocument::Compact));
    if (!reply) {
        return false;
    }

    streamReply_ = reply;
    streamBuffer_.clear();
    streamAccumulated_.clear();
    streamCanceled_ = false;
    streamRunning_ = true;
    streamFormat_ = StreamFormat::OllamaNdjson;
    streamTimeout_.start(timeoutMs_);

    connect(reply, &QNetworkReply::readyRead, this, &AiAssistantService::handleStreamReadyRead);
    connect(reply, &QNetworkReply::finished, this, [this]() {
        if (!streamReply_) {
            return;
        }
        handleStreamReadyRead();

        AiResult result;
        if (streamCanceled_) {
            result.success = false;
            result.message = "已取消生成";
            finalizeStreamResult(result);
            return;
        }

        const QString networkError = streamReply_->error() == QNetworkReply::NoError ? QString() : streamReply_->errorString();
        if (!networkError.isEmpty()) {
            result.success = false;
            result.message = networkError;
            finalizeStreamResult(result);
            return;
        }
        result.success = true;
        result.content = streamAccumulated_;
        if (result.content.trimmed().isEmpty()) {
            result.success = false;
            result.message = "模型未返回内容";
        }
        finalizeStreamResult(result);
    });
    return true;
}

void AiAssistantService::cancelStream() {
    if (!streamRunning_) {
        return;
    }
    streamCanceled_ = true;
    if (streamReply_ && streamReply_->isRunning()) {
        streamReply_->abort();
    } else {
        AiResult result;
        result.success = false;
        result.message = "已取消生成";
        finalizeStreamResult(result);
    }
}

void AiAssistantService::handleStreamReadyRead() {
    if (!streamReply_) {
        return;
    }
    streamBuffer_.append(streamReply_->readAll());

    if (streamFormat_ == StreamFormat::Sse) {
        while (true) {
            const int newline = streamBuffer_.indexOf('\n');
            if (newline < 0) {
                break;
            }
            QByteArray line = streamBuffer_.left(newline).trimmed();
            streamBuffer_.remove(0, newline + 1);
            if (line.isEmpty()) {
                continue;
            }
            if (!line.startsWith("data:")) {
                continue;
            }
            line = line.mid(5).trimmed();
            if (line == "[DONE]") {
                continue;
            }
            const QJsonDocument doc = QJsonDocument::fromJson(line);
            if (!doc.isObject()) {
                continue;
            }
            const QJsonObject obj = doc.object();
            const QString err = useClaudeProvider() ? extractClaudeError(obj) : extractZhipuError(obj);
            if (!err.isEmpty()) {
                AiResult result;
                result.success = false;
                result.message = err;
                finalizeStreamResult(result);
                return;
            }
            const QString piece = useClaudeProvider() ? extractClaudeContent(obj) : extractZhipuContent(obj);
            if (!piece.isEmpty()) {
                streamAccumulated_.append(piece);
                emit streamChunk(piece);
            }
        }
        return;
    }

    while (true) {
        const int newline = streamBuffer_.indexOf('\n');
        if (newline < 0) {
            break;
        }
        const QByteArray line = streamBuffer_.left(newline).trimmed();
        streamBuffer_.remove(0, newline + 1);
        if (line.isEmpty()) {
            continue;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(line);
        if (!doc.isObject()) {
            continue;
        }
        const QJsonObject obj = doc.object();
        const QString err = obj.value("error").toString();
        if (!err.isEmpty()) {
            AiResult result;
            result.success = false;
            result.message = err;
            finalizeStreamResult(result);
            return;
        }
        const QString piece = obj.value("response").toString();
        if (!piece.isEmpty()) {
            streamAccumulated_.append(piece);
            emit streamChunk(piece);
        }
    }
}

void AiAssistantService::finalizeStreamResult(const AiResult &result) {
    if (!streamRunning_) {
        return;
    }
    streamTimeout_.stop();
    streamRunning_ = false;
    streamCanceled_ = false;

    if (streamReply_) {
        disconnect(streamReply_, nullptr, this, nullptr);
        streamReply_->deleteLater();
        streamReply_.clear();
    }
    streamBuffer_.clear();
    streamAccumulated_.clear();
    emit streamCompleted(result);
}

AiResult AiAssistantService::modelStatus() {
    if (useCloudProvider()) {
        AiResult result;
        const QString resolvedModel = modelTag_;
        if (zhipuApiKey_.isEmpty()) {
            result.success = false;
            result.message = "云端 API Key 未配置";
            return result;
        }
        QNetworkRequest request(QUrl(zhipuBaseUrl_ + "/models"));
        if (useClaudeProvider()) {
            request.setRawHeader("x-api-key", zhipuApiKey_.toUtf8());
            request.setRawHeader("anthropic-version", "2023-06-01");
        } else {
            request.setRawHeader("Authorization", ("Bearer " + zhipuApiKey_).toUtf8());
        }
        QNetworkReply *reply = network_.get(request);
        if (!reply) {
            result.success = false;
            result.message = "网络请求创建失败";
            return result;
        }

        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        QObject::connect(&timer, &QTimer::timeout, &loop, [&]() {
            if (reply && reply->isRunning()) {
                reply->abort();
            }
            loop.quit();
        });
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        timer.start(timeoutMs_);
        loop.exec();

        const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray responseBytes = reply->readAll();
        const QString networkError = reply->error() == QNetworkReply::NoError ? QString() : reply->errorString();
        reply->deleteLater();
        if (!networkError.isEmpty()) {
            result.success = false;
            result.message = networkError;
            return result;
        }
        if (statusCode >= 400 || statusCode == 0) {
            result.success = false;
            result.message = QString("HTTP %1").arg(statusCode);
            return result;
        }
        const QJsonDocument doc = QJsonDocument::fromJson(responseBytes);
        const QJsonArray data = doc.object().value("data").toArray();
        QJsonArray models;
        bool ready = false;
        for (const QJsonValue &v : data) {
            const QString id = v.toObject().value("id").toString();
            if (!id.isEmpty()) {
                models.append(id);
            }
            if (id == resolvedModel) {
                ready = true;
            }
        }
        if (models.isEmpty()) {
            models.append(resolvedModel);
        }
        result.success = true;
        result.payload = QJsonObject{
            {"ready", true},
            {"model", resolvedModel},
            {"models", models},
            {"provider", provider_},
        };
        result.message = ready ? "云端模型已就绪" : "云端连接正常";
        return result;
    }

    AiResult result = requestJson(QUrl(baseUrl_ + "/api/tags"), QJsonObject(), "GET");
    if (!result.success) {
        return result;
    }
    const QJsonArray models = result.payload.value("models").toArray();
    bool ready = false;
    QJsonArray names;
    for (const QJsonValue &item : models) {
        const QJsonObject obj = item.toObject();
        const QString name = obj.value("name").toString();
        if (!name.isEmpty()) {
            names.append(name);
        }
        if (name == modelTag_) {
            ready = true;
        }
    }
    result.payload = QJsonObject{
        {"ready", ready},
        {"model", modelTag_},
        {"models", names},
        {"provider", "local_ollama"},
    };
    result.message = ready ? "模型已就绪" : "模型未安装";
    return result;
}

AiResult AiAssistantService::pullModel(const QString &modelTag) {
    if (useCloudProvider()) {
        AiResult result;
        result.success = true;
        result.message = "云端模式无需下载本地模型";
        result.content = result.message;
        return result;
    }
    const QString resolvedModel = modelTag.trimmed().isEmpty() ? modelTag_ : modelTag.trimmed();
    const QJsonObject payload{
        {"model", resolvedModel},
        {"stream", false},
    };
    AiResult result = requestJson(QUrl(baseUrl_ + "/api/pull"), payload, "POST");
    if (!result.success) {
        return result;
    }
    const QString error = result.payload.value("error").toString();
    if (!error.isEmpty()) {
        result.success = false;
        result.message = error;
        return result;
    }
    result.message = result.payload.value("status").toString("模型拉取完成");
    result.content = result.message;
    return result;
}

AiResult AiAssistantService::summarizeTune(const InstrumentStatus &status, const SpectrumFrame &frame,
                                           const TuneParameters &tune, const ScanSettings &scan) {
    const QJsonObject ctx = buildTuneContext(status, frame, tune, scan);
    const QString prompt =
        "你是四极杆质谱调谐助手。必须包含【思考过程】和【结论】两个部分，并严格按以下结构输出中文：\n"
        "【思考过程】\n"
        "- 先解释你如何解读关键数据（可分点）\n"
        "- 指出不确定性与假设\n"
        "【结论】\n"
        "1) 当前状态总结\n2) 关键异常/风险\n3) 建议参数调整（给出方向和幅度）\n4) 下一步操作。\n"
        "若数据不足要明确指出。\n\nJSON:\n" + QString::fromUtf8(QJsonDocument(ctx).toJson(QJsonDocument::Compact));
    return requestGenerate(prompt, modelTag_);
}

AiResult AiAssistantService::troubleshootTune(const InstrumentStatus &status, const SpectrumFrame &frame,
                                              const TuneParameters &tune, const ScanSettings &scan,
                                              const QString &question) {
    const QJsonObject ctx = buildTuneContext(status, frame, tune, scan);
    const QString prompt =
        "你是四极杆质谱故障诊断助手。用户问题：" + question +
        "\n必须包含【思考过程】和【结论】两个部分，并输出固定结构：\n"
        "【思考过程】\n"
        "- 对问题进行原因推理链路说明（分点）\n"
        "- 给出你排除其它原因的依据\n"
        "【结论】\n"
        "1) 可能原因（最多3条）\n2) 排查步骤（可执行）\n3) 参数建议（含预期影响）\n4) 风险提示。\n"
        "结合以下 JSON：\n" + QString::fromUtf8(QJsonDocument(ctx).toJson(QJsonDocument::Compact));
    return requestGenerate(prompt, modelTag_);
}

SettingsService::SettingsService(SettingsRepository *settingsRepository, QObject *parent)
    : QObject(parent), settingsRepository_(settingsRepository) {}

DataProcessingSettings SettingsService::dataProcessingSettings() const {
    return settingsRepository_->loadDataProcessingSettings();
}

void SettingsService::saveDataProcessingSettings(const DataProcessingSettings &settings) {
    settingsRepository_->saveDataProcessingSettings(settings);
}

SystemSettings SettingsService::systemSettings() const {
    return settingsRepository_->loadSystemSettings();
}

void SettingsService::saveSystemSettings(const SystemSettings &settings) {
    settingsRepository_->saveSystemSettings(settings);
}

ChartSettings SettingsService::chartSettings() const {
    return settingsRepository_->loadChartSettings();
}

void SettingsService::saveChartSettings(const ChartSettings &settings) {
    settingsRepository_->saveChartSettings(settings);
}

InstrumentControlSettings SettingsService::instrumentControlSettings() const {
    return settingsRepository_->loadInstrumentControlSettings();
}

void SettingsService::saveInstrumentControlSettings(const InstrumentControlSettings &settings) {
    settingsRepository_->saveInstrumentControlSettings(settings);
}

QJsonObject SettingsService::aiChatState() const {
    return settingsRepository_->loadAiChatState();
}

void SettingsService::saveAiChatState(const QJsonObject &state) const {
    settingsRepository_->saveAiChatState(state);
}

QJsonObject SettingsService::aiProviderConfig() const {
    return settingsRepository_->loadAiProviderConfig();
}

void SettingsService::saveAiProviderConfig(const QJsonObject &config) const {
    settingsRepository_->saveAiProviderConfig(config);
}

QJsonObject SettingsService::aiSummaryHistory() const {
    return settingsRepository_->loadAiSummaryHistory();
}

void SettingsService::saveAiSummaryHistory(const QJsonObject &history) const {
    settingsRepository_->saveAiSummaryHistory(history);
}

}  // namespace deviceapp
