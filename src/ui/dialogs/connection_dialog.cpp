#include "ui/dialogs/connection_dialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QVBoxLayout>

namespace deviceapp {

ConnectionDialog::ConnectionDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("仪器连接");
    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();

    hostEdit_ = new QLineEdit(this);
    portSpin_ = new QSpinBox(this);
    portSpin_->setRange(1, 65535);
    portSpin_->setValue(9000);

    form->addRow("IP", hostEdit_);
    form->addRow("端口", portSpin_);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

void ConnectionDialog::setConfig(const DeviceConnectionConfig &config) {
    hostEdit_->setText(config.host);
    portSpin_->setValue(config.port);
}

DeviceConnectionConfig ConnectionDialog::config() const {
    DeviceConnectionConfig config;
    config.host = hostEdit_->text().trimmed().isEmpty() ? "127.0.0.1" : hostEdit_->text().trimmed();
    config.port = static_cast<quint16>(portSpin_->value());
    return config;
}

}  // namespace deviceapp
