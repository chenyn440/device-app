#pragma once

#include <QHash>
#include <QPointer>
#include <QTcpServer>
#include <QTcpSocket>

#include "app/application_context.h"

namespace deviceapp {

class GatewayServer : public QObject {
    Q_OBJECT

public:
    explicit GatewayServer(quint16 port, QString webRoot, QObject *parent = nullptr);
    bool start(QString *errorMessage = nullptr);

private slots:
    void onNewConnection();
    void onSocketReadyRead();
    void onSocketDisconnected();

private:
    struct HttpRequest {
        QString method;
        QString path;
        QString version;
        QHash<QString, QString> headers;
        QByteArray body;
    };

    bool tryParseRequest(QByteArray *buffer, HttpRequest *request) const;
    void handleRequest(QTcpSocket *socket, const HttpRequest &request);

    bool validateAuth(const HttpRequest &request) const;

    void handleApiRequest(QTcpSocket *socket, const HttpRequest &request);
    void handleStaticRequest(QTcpSocket *socket, const HttpRequest &request);

    void writeJsonResponse(QTcpSocket *socket, int statusCode, const QJsonObject &payload, bool close = true) const;
    void writeTextResponse(QTcpSocket *socket, int statusCode, const QByteArray &contentType, const QByteArray &body,
                           bool close = true) const;
    void writeNoContent(QTcpSocket *socket) const;
    void writeUnauthorized(QTcpSocket *socket) const;

    void attachSseClient(QTcpSocket *socket);
    void broadcastEvent(const QString &eventName, const QJsonObject &payload);
    void sendStatusEvent(QTcpSocket *socket) const;
    void sendFrameEvent(QTcpSocket *socket) const;

    static QByteArray statusText(int statusCode);
    static QByteArray mimeTypeForPath(const QString &path);
    static QJsonObject okEnvelope(const QJsonValue &data = QJsonValue());
    static QJsonObject errorEnvelope(const QString &message);

    QTcpServer server_;
    quint16 port_;
    QString webRoot_;
    QString authToken_;

    ApplicationContext context_;
    InstrumentStatus latestStatus_;
    SpectrumFrame latestFrame_;
    bool hasFrame_ = false;

    QHash<QTcpSocket *, QByteArray> socketBuffers_;
    QList<QPointer<QTcpSocket>> sseClients_;
};

}  // namespace deviceapp
