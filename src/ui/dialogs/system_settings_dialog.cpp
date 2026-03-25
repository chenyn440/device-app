#include "ui/dialogs/system_settings_dialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace deviceapp {

SystemSettingsDialog::SystemSettingsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("系统设置");
    resize(520, 420);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);

    auto *rfdaGroup = new QGroupBox("RFDA 设置", this);
    auto *rfdaLayout = new QFormLayout(rfdaGroup);
    rfddsEdit_ = new QLineEdit(this);
    applyRfddsButton_ = new QPushButton("设置", this);
    auto *rfddsRow = new QHBoxLayout();
    rfddsRow->addWidget(rfddsEdit_);
    rfddsRow->addWidget(applyRfddsButton_);
    auto *rfddsWidget = new QWidget(this);
    rfddsWidget->setLayout(rfddsRow);
    rfdaLayout->addRow("RFDDS", rfddsWidget);
    layout->addWidget(rfdaGroup);

    auto *systemInfoGroup = new QGroupBox("系统信息", this);
    auto *systemInfoLayout = new QFormLayout(systemInfoGroup);
    serialNumberEdit_ = new QLineEdit(this);
    modelBox_ = new QComboBox(this);
    modelBox_->addItems({"QMS-100", "QMS-200", "QMS-300"});
    applySystemInfoButton_ = new QPushButton("设置", this);
    auto *systemButtonRow = new QHBoxLayout();
    systemButtonRow->addStretch();
    systemButtonRow->addWidget(applySystemInfoButton_);
    auto *systemButtonWidget = new QWidget(this);
    systemButtonWidget->setLayout(systemButtonRow);
    systemInfoLayout->addRow("编号", serialNumberEdit_);
    systemInfoLayout->addRow("型号", modelBox_);
    systemInfoLayout->addRow("", systemButtonWidget);
    layout->addWidget(systemInfoGroup);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);

    connect(applyRfddsButton_, &QPushButton::clicked, this, [this]() {
        rfddsEdit_->setText(rfddsEdit_->text().trimmed());
    });
    connect(applySystemInfoButton_, &QPushButton::clicked, this, [this]() {
        serialNumberEdit_->setText(serialNumberEdit_->text().trimmed());
    });
}

void SystemSettingsDialog::setSettings(const SystemSettings &settings) {
    rfddsEdit_->setText(settings.rfddsValue);
    serialNumberEdit_->setText(settings.systemSerialNumber);
    modelBox_->setCurrentText(settings.systemModel);
}

SystemSettings SystemSettingsDialog::settings() const {
    SystemSettings settings;
    settings.rfddsValue = rfddsEdit_->text().trimmed();
    settings.systemSerialNumber = serialNumberEdit_->text().trimmed();
    settings.systemModel = modelBox_->currentText();
    return settings;
}

}  // namespace deviceapp
