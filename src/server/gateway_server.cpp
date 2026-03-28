#include "server/gateway_server.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

#include <algorithm>
#include <utility>

namespace deviceapp {

namespace {

QString normalizedPath(QString path) {
    if (path.isEmpty()) {
        return "/";
    }
    const int queryIndex = path.indexOf('?');
    if (queryIndex >= 0) {
        path = path.left(queryIndex);
    }
    if (!path.startsWith('/')) {
        path.prepend('/');
    }
    return path;
}

bool isPathSafe(const QString &path) {
    return !path.contains("..") && !path.contains('\\');
}

}  // namespace

GatewayServer::GatewayServer(quint16 port, QString webRoot, QObject *parent)
    : QObject(parent), port_(port), webRoot_(std::move(webRoot)) {
    authToken_ = qEnvironmentVariable("DEVICE_APP_API_TOKEN");

    connect(&server_, &QTcpServer::newConnection, this, &GatewayServer::onNewConnection);

    connect(context_.deviceAdapter.get(), &IDeviceAdapter::statusUpdated, this, [this](const InstrumentStatus &status) {
        latestStatus_ = status;
        context_.scanControlService->updateInstrumentStatus(status);
        broadcastEvent("status", toJson(status));
    });
    connect(context_.deviceAdapter.get(), &IDeviceAdapter::frameUpdated, this, [this](const SpectrumFrame &frame) {
        latestFrame_ = frame;
        hasFrame_ = true;
        broadcastEvent("frame", toJson(frame));
    });
    connect(context_.deviceAdapter.get(), &IDeviceAdapter::errorOccurred, this, [this](const QString &message) {
        broadcastEvent("error", QJsonObject{{"message", message}});
    });

    context_.deviceAdapter->readStatusSnapshot();
}

bool GatewayServer::start(QString *errorMessage) {
    if (!server_.listen(QHostAddress::LocalHost, port_)) {
        if (errorMessage) {
            *errorMessage = server_.errorString();
        }
        return false;
    }
    qInfo().noquote() << "Gateway listening on port" << server_.serverPort();
    qInfo().noquote() << "Web root:" << webRoot_;
    return true;
}

void GatewayServer::onNewConnection() {
    while (QTcpSocket *socket = server_.nextPendingConnection()) {
        socketBuffers_.insert(socket, QByteArray());
        connect(socket, &QTcpSocket::readyRead, this, &GatewayServer::onSocketReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &GatewayServer::onSocketDisconnected);
    }
}

void GatewayServer::onSocketReadyRead() {
    auto *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) {
        return;
    }

    socketBuffers_[socket].append(socket->readAll());
    HttpRequest request;
    while (tryParseRequest(&socketBuffers_[socket], &request)) {
        handleRequest(socket, request);
        if (!socket->isOpen()) {
            socketBuffers_.remove(socket);
            return;
        }
    }
}

void GatewayServer::onSocketDisconnected() {
    auto *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) {
        return;
    }
    socketBuffers_.remove(socket);
    sseClients_.erase(std::remove_if(sseClients_.begin(), sseClients_.end(), [socket](const QPointer<QTcpSocket> &ptr) {
        return !ptr || ptr.data() == socket;
    }), sseClients_.end());
    socket->deleteLater();
}

bool GatewayServer::tryParseRequest(QByteArray *buffer, HttpRequest *request) const {
    const int headerEnd = buffer->indexOf("\r\n\r\n");
    if (headerEnd < 0) {
        return false;
    }

    const QByteArray headerBytes = buffer->left(headerEnd);
    const QList<QByteArray> lines = headerBytes.split('\n');
    if (lines.isEmpty()) {
        return false;
    }

    const QList<QByteArray> startLine = lines.first().trimmed().split(' ');
    if (startLine.size() < 3) {
        return false;
    }

    request->method = QString::fromUtf8(startLine[0]).trimmed().toUpper();
    request->path = QString::fromUtf8(startLine[1]).trimmed();
    request->version = QString::fromUtf8(startLine[2]).trimmed();
    request->headers.clear();

    int contentLength = 0;
    for (int i = 1; i < lines.size(); ++i) {
        const QByteArray line = lines[i].trimmed();
        const int sep = line.indexOf(':');
        if (sep <= 0) {
            continue;
        }
        const QString key = QString::fromUtf8(line.left(sep)).trimmed().toLower();
        const QString value = QString::fromUtf8(line.mid(sep + 1)).trimmed();
        request->headers.insert(key, value);
        if (key == "content-length") {
            contentLength = value.toInt();
        }
    }

    const int fullRequestSize = headerEnd + 4 + contentLength;
    if (buffer->size() < fullRequestSize) {
        return false;
    }

    request->body = buffer->mid(headerEnd + 4, contentLength);
    buffer->remove(0, fullRequestSize);
    request->path = normalizedPath(request->path);
    return true;
}

void GatewayServer::handleRequest(QTcpSocket *socket, const HttpRequest &request) {
    if (request.method == "OPTIONS") {
        writeNoContent(socket);
        return;
    }

    if (request.path.startsWith("/api/")) {
        if (!validateAuth(request)) {
            writeUnauthorized(socket);
            return;
        }
        handleApiRequest(socket, request);
        return;
    }

    if (request.path == "/health") {
        writeJsonResponse(socket, 200, okEnvelope(QJsonObject{{"status", "ok"}}));
        return;
    }

    handleStaticRequest(socket, request);
}

bool GatewayServer::validateAuth(const HttpRequest &request) const {
    if (authToken_.isEmpty()) {
        return true;
    }
    const QString authorization = request.headers.value("authorization");
    return authorization == QString("Bearer %1").arg(authToken_);
}

void GatewayServer::handleApiRequest(QTcpSocket *socket, const HttpRequest &request) {
    const auto bodyJson = [request]() -> QJsonObject {
        if (request.body.isEmpty()) {
            return {};
        }
        const QJsonDocument document = QJsonDocument::fromJson(request.body);
        return document.object();
    };

    if (request.path == "/api/status" && request.method == "GET") {
        QJsonObject data{{"status", toJson(latestStatus_)}};
        if (hasFrame_) {
            data["frame"] = toJson(latestFrame_);
        }
        writeJsonResponse(socket, 200, okEnvelope(data));
        return;
    }

    if (request.path == "/api/stream" && request.method == "GET") {
        attachSseClient(socket);
        sendStatusEvent(socket);
        sendFrameEvent(socket);
        return;
    }

    if (request.path == "/api/connection/connect" && request.method == "POST") {
        const DeviceConnectionConfig config = connectionConfigFromJson(bodyJson());
        context_.connectionService->connectToDevice(config);
        writeJsonResponse(socket, 200, okEnvelope(QJsonObject{{"connected", true}}));
        return;
    }

    if (request.path == "/api/connection/disconnect" && request.method == "POST") {
        context_.connectionService->disconnectFromDevice();
        writeJsonResponse(socket, 200, okEnvelope(QJsonObject{{"connected", false}}));
        return;
    }

    if (request.path == "/api/scan/start" && request.method == "POST") {
        const QJsonObject payload = bodyJson();
        const QJsonObject scanJson = payload.contains("scanSettings") ? payload.value("scanSettings").toObject() : payload;
        const ScanSettings settings = scanSettingsFromJson(scanJson);
        QString validationError;
        if (!context_.scanControlService->canStartScan(settings, &validationError)) {
            writeJsonResponse(socket, 400, errorEnvelope(validationError));
            return;
        }
        context_.scanControlService->startScan(settings);
        writeJsonResponse(socket, 200, okEnvelope(QJsonObject{{"scanning", true}}));
        return;
    }

    if (request.path == "/api/scan/stop" && request.method == "POST") {
        context_.scanControlService->stopScan();
        writeJsonResponse(socket, 200, okEnvelope(QJsonObject{{"scanning", false}}));
        return;
    }

    if (request.path == "/api/tune/apply" && request.method == "POST") {
        const TuneParameters parameters = tuneParametersFromJson(bodyJson());
        context_.tuneService->applyTuneParameters(parameters);
        writeJsonResponse(socket, 200, okEnvelope(QJsonObject{{"applied", true}}));
        return;
    }

    if (request.path == "/api/monitor/method/save" && request.method == "POST") {
        const MonitorMethod method = monitorMethodFromJson(bodyJson());
        const QString path = context_.monitorService->saveMethod(method);
        writeJsonResponse(socket, 200, okEnvelope(QJsonObject{{"path", path}}));
        return;
    }

    if (request.path == "/api/monitor/method/load" && request.method == "POST") {
        const QString path = bodyJson().value("path").toString();
        if (path.isEmpty()) {
            writeJsonResponse(socket, 400, errorEnvelope("path is required"));
            return;
        }
        const MonitorMethod method = context_.monitorService->loadMethod(path);
        writeJsonResponse(socket, 200, okEnvelope(toJson(method)));
        return;
    }

    if (request.path == "/api/monitor/method/list" && request.method == "GET") {
        const QStringList methods = context_.monitorService->listMethods();
        QJsonArray array;
        for (const QString &method : methods) {
            array.append(method);
        }
        writeJsonResponse(socket, 200, okEnvelope(array));
        return;
    }

    if (request.path == "/api/frame/save" && request.method == "POST") {
        if (!hasFrame_) {
            writeJsonResponse(socket, 400, errorEnvelope("no frame available"));
            return;
        }
        const QString path = context_.persistenceService->saveFrame(latestFrame_);
        writeJsonResponse(socket, 200, okEnvelope(QJsonObject{{"path", path}}));
        return;
    }

    writeJsonResponse(socket, 404, errorEnvelope("not found"));
}

void GatewayServer::handleStaticRequest(QTcpSocket *socket, const HttpRequest &request) {
    QString relativePath = request.path;
    if (relativePath == "/") {
        relativePath = "/index.html";
    }
    if (!isPathSafe(relativePath)) {
        writeJsonResponse(socket, 400, errorEnvelope("invalid path"));
        return;
    }

    const QString fullPath = webRoot_ + relativePath;
    QFile file(fullPath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        writeJsonResponse(socket, 404, errorEnvelope("not found"));
        return;
    }

    writeTextResponse(socket, 200, mimeTypeForPath(fullPath), file.readAll());
}

void GatewayServer::writeJsonResponse(QTcpSocket *socket, int statusCode, const QJsonObject &payload, bool close) const {
    const QByteArray body = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    writeTextResponse(socket, statusCode, "application/json; charset=utf-8", body, close);
}

void GatewayServer::writeTextResponse(QTcpSocket *socket, int statusCode, const QByteArray &contentType, const QByteArray &body,
                                      bool close) const {
    if (!socket || !socket->isOpen()) {
        return;
    }

    QByteArray response;
    response += "HTTP/1.1 " + QByteArray::number(statusCode) + " " + statusText(statusCode) + "\r\n";
    response += "Content-Type: " + contentType + "\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
    response += "Access-Control-Allow-Methods: GET,POST,OPTIONS\r\n";
    response += "Content-Length: " + QByteArray::number(body.size()) + "\r\n";
    response += close ? "Connection: close\r\n" : "Connection: keep-alive\r\n";
    response += "\r\n";
    response += body;
    socket->write(response);
    socket->flush();
    if (close) {
        socket->disconnectFromHost();
    }
}

void GatewayServer::writeNoContent(QTcpSocket *socket) const {
    if (!socket || !socket->isOpen()) {
        return;
    }
    QByteArray response;
    response += "HTTP/1.1 204 No Content\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
    response += "Access-Control-Allow-Methods: GET,POST,OPTIONS\r\n";
    response += "Content-Length: 0\r\n";
    response += "Connection: close\r\n\r\n";
    socket->write(response);
    socket->flush();
    socket->disconnectFromHost();
}

void GatewayServer::writeUnauthorized(QTcpSocket *socket) const {
    writeJsonResponse(socket, 401, errorEnvelope("unauthorized"));
}

void GatewayServer::attachSseClient(QTcpSocket *socket) {
    if (!socket || !socket->isOpen()) {
        return;
    }

    QByteArray response;
    response += "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/event-stream\r\n";
    response += "Cache-Control: no-cache\r\n";
    response += "Connection: keep-alive\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "\r\n";
    socket->write(response);
    socket->flush();

    sseClients_.append(socket);
}

void GatewayServer::broadcastEvent(const QString &eventName, const QJsonObject &payload) {
    const QByteArray data = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    const QByteArray packet = "event: " + eventName.toUtf8() + "\n" + "data: " + data + "\n\n";

    sseClients_.erase(std::remove_if(sseClients_.begin(), sseClients_.end(), [&](const QPointer<QTcpSocket> &ptr) {
        if (!ptr || !ptr->isOpen()) {
            return true;
        }
        ptr->write(packet);
        ptr->flush();
        return false;
    }), sseClients_.end());
}

void GatewayServer::sendStatusEvent(QTcpSocket *socket) const {
    if (!socket || !socket->isOpen()) {
        return;
    }
    const QByteArray data = QJsonDocument(toJson(latestStatus_)).toJson(QJsonDocument::Compact);
    socket->write("event: status\n");
    socket->write("data: " + data + "\n\n");
    socket->flush();
}

void GatewayServer::sendFrameEvent(QTcpSocket *socket) const {
    if (!socket || !socket->isOpen() || !hasFrame_) {
        return;
    }
    const QByteArray data = QJsonDocument(toJson(latestFrame_)).toJson(QJsonDocument::Compact);
    socket->write("event: frame\n");
    socket->write("data: " + data + "\n\n");
    socket->flush();
}

QByteArray GatewayServer::statusText(int statusCode) {
    switch (statusCode) {
    case 200:
        return "OK";
    case 204:
        return "No Content";
    case 400:
        return "Bad Request";
    case 401:
        return "Unauthorized";
    case 404:
        return "Not Found";
    default:
        return "Internal Server Error";
    }
}

QByteArray GatewayServer::mimeTypeForPath(const QString &path) {
    if (path.endsWith(".html")) {
        return "text/html; charset=utf-8";
    }
    if (path.endsWith(".js")) {
        return "application/javascript; charset=utf-8";
    }
    if (path.endsWith(".css")) {
        return "text/css; charset=utf-8";
    }
    if (path.endsWith(".json")) {
        return "application/json; charset=utf-8";
    }
    return "application/octet-stream";
}

QJsonObject GatewayServer::okEnvelope(const QJsonValue &data) {
    QJsonObject envelope{{"ok", true}};
    if (!data.isUndefined()) {
        envelope.insert("data", data);
    }
    return envelope;
}

QJsonObject GatewayServer::errorEnvelope(const QString &message) {
    return QJsonObject{{"ok", false}, {"error", message}};
}

}  // namespace deviceapp
