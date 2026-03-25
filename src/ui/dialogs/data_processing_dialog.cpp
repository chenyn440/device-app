#include "ui/dialogs/data_processing_dialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QVBoxLayout>

namespace deviceapp {

DataProcessingDialog::DataProcessingDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("数据处理");

    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();
    smoothingCheck_ = new QCheckBox("启用平滑滤波", this);
    windowSpin_ = new QSpinBox(this);
    windowSpin_->setRange(1, 99);
    windowSpin_->setSingleStep(2);
    lambdaSpin_ = new QDoubleSpinBox(this);
    lambdaSpin_->setRange(1.0, 1000000.0);
    lambdaSpin_->setDecimals(2);
    lambdaSpin_->setValue(1000.0);
    asymmetrySpin_ = new QDoubleSpinBox(this);
    asymmetrySpin_->setRange(0.0001, 1.0);
    asymmetrySpin_->setDecimals(4);
    asymmetrySpin_->setSingleStep(0.001);
    form->addRow("", smoothingCheck_);
    form->addRow("平滑窗口", windowSpin_);
    form->addRow("基线 Lambda", lambdaSpin_);
    form->addRow("基线 Asymmetry", asymmetrySpin_);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

void DataProcessingDialog::setSettings(const DataProcessingSettings &settings) {
    smoothingCheck_->setChecked(settings.smoothingEnabled);
    windowSpin_->setValue(settings.smoothingWindow);
    lambdaSpin_->setValue(settings.baselineLambda);
    asymmetrySpin_->setValue(settings.baselineAsymmetry);
}

DataProcessingSettings DataProcessingDialog::settings() const {
    DataProcessingSettings settings;
    settings.smoothingEnabled = smoothingCheck_->isChecked();
    settings.smoothingWindow = windowSpin_->value();
    settings.baselineLambda = lambdaSpin_->value();
    settings.baselineAsymmetry = asymmetrySpin_->value();
    return settings;
}

}  // namespace deviceapp
