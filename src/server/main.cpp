#include <QCoreApplication>
#include <QDir>

#include "server/gateway_server.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("device-gateway");
    QCoreApplication::setOrganizationName("deviceapp");

    bool ok = false;
    const quint16 port = qEnvironmentVariableIntValue("DEVICE_APP_GATEWAY_PORT", &ok);
    const quint16 listenPort = ok ? port : 8787;

    QString webRoot = qEnvironmentVariable("DEVICE_APP_WEB_ROOT");
    if (webRoot.isEmpty()) {
        webRoot = QDir::currentPath() + "/web";
    }

    deviceapp::GatewayServer gateway(listenPort, webRoot);
    QString errorMessage;
    if (!gateway.start(&errorMessage)) {
        qCritical().noquote() << "Failed to start gateway:" << errorMessage;
        return 1;
    }

    return app.exec();
}
