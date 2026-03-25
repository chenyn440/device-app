#include <QApplication>

#include "app/application_context.h"
#include "ui/main_window.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("device-app");
    QApplication::setOrganizationName("deviceapp");

    deviceapp::ApplicationContext context;
    deviceapp::MainWindow window(&context);
    window.resize(1600, 900);
    window.show();

    return QApplication::exec();
}
