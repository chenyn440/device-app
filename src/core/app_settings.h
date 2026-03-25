#pragma once

#include <QDir>

namespace deviceapp {

class AppSettings {
public:
    static QString appDataRoot();
    static QString configPath();
    static QString frameDirectory();
    static QString monitorMethodDirectory();
    static QString tuneDirectory();
    static void ensureDirectories();
};

}  // namespace deviceapp
