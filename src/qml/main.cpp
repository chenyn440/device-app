#include <QApplication>
#include <QCoreApplication>
#include <QFont>
#include <QFontDatabase>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "qml/app_state.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QGuiApplication::setApplicationName("device-app");
    QGuiApplication::setOrganizationName("deviceapp");
    QQuickStyle::setStyle("Basic");

    const int fontId = QFontDatabase::addApplicationFont(":/qml/fonts/HiraginoSansGB.ttc");
    if (fontId >= 0) {
        const QStringList families = QFontDatabase::applicationFontFamilies(fontId);
        if (!families.isEmpty()) {
            QFont appFont = app.font();
            appFont.setFamily(families.first());
            QGuiApplication::setFont(appFont);
        }
    }

    QQmlApplicationEngine engine;
    deviceapp::AppState appState;
    engine.rootContext()->setContextProperty("appState", static_cast<QObject *>(&appState));

    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, []() { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);
    engine.load(url);

    return QGuiApplication::exec();
}
