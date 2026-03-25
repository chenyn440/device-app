#pragma once

#include <QDialog>

#include "core/types.h"

class QLineEdit;
class QSpinBox;

namespace deviceapp {

class ConnectionDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConnectionDialog(QWidget *parent = nullptr);

    void setConfig(const DeviceConnectionConfig &config);
    DeviceConnectionConfig config() const;

private:
    QLineEdit *hostEdit_;
    QSpinBox *portSpin_;
};

}  // namespace deviceapp
