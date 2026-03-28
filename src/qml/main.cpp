#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "qml/app_state.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("device-app");
    QApplication::setOrganizationName("deviceapp");
    QQuickStyle::setStyle("Basic");

    QQmlApplicationEngine engine;
    deviceapp::AppState appState;
    engine.rootContext()->setContextProperty("appState", static_cast<QObject *>(&appState));

    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, []() { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);
    engine.load(url);

    return QApplication::exec();
}
