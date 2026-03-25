#include "ui/dialogs/instrument_control_dialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QVBoxLayout>

namespace deviceapp {

InstrumentControlDialog::InstrumentControlDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("仪器控制");
    resize(420, 360);

    auto *layout = new QVBoxLayout(this);

    auto *switchBox = new QGroupBox("开关控制", this);
    auto *switchLayout = new QVBoxLayout(switchBox);
    forePumpCheck_ = new QCheckBox("前级泵", this);
    foreValveCheck_ = new QCheckBox("前级阀", this);
    molecularPumpCheck_ = new QCheckBox("分子泵", this);
    inletValveCheck_ = new QCheckBox("进样阀", this);
    filamentCheck_ = new QCheckBox("灯丝", this);
    multiplierCheck_ = new QCheckBox("倍增器", this);
    switchLayout->addWidget(forePumpCheck_);
    switchLayout->addWidget(foreValveCheck_);
    switchLayout->addWidget(molecularPumpCheck_);
    switchLayout->addWidget(inletValveCheck_);
    switchLayout->addWidget(filamentCheck_);
    switchLayout->addWidget(multiplierCheck_);

    auto *temperatureBox = new QGroupBox("温度控制", this);
    auto *temperatureLayout = new QFormLayout(temperatureBox);
    sourceTempSpin_ = new QDoubleSpinBox(this);
    sourceTempSpin_->setRange(0.0, 500.0);
    chamberTempSpin_ = new QDoubleSpinBox(this);
    chamberTempSpin_->setRange(0.0, 200.0);
    temperatureLayout->addRow("离子源目标温度", sourceTempSpin_);
    temperatureLayout->addRow("腔体目标温度", chamberTempSpin_);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    layout->addWidget(switchBox);
    layout->addWidget(temperatureBox);
    layout->addWidget(buttons);
}

void InstrumentControlDialog::setSettings(const InstrumentControlSettings &settings) {
    forePumpCheck_->setChecked(settings.forePumpEnabled);
    foreValveCheck_->setChecked(settings.foreValveEnabled);
    molecularPumpCheck_->setChecked(settings.molecularPumpEnabled);
    inletValveCheck_->setChecked(settings.inletValveEnabled);
    filamentCheck_->setChecked(settings.filamentEnabled);
    multiplierCheck_->setChecked(settings.multiplierEnabled);
    sourceTempSpin_->setValue(settings.targetSourceTemperature);
    chamberTempSpin_->setValue(settings.targetChamberTemperature);
}

InstrumentControlSettings InstrumentControlDialog::settings() const {
    InstrumentControlSettings settings;
    settings.forePumpEnabled = forePumpCheck_->isChecked();
    settings.foreValveEnabled = foreValveCheck_->isChecked();
    settings.molecularPumpEnabled = molecularPumpCheck_->isChecked();
    settings.inletValveEnabled = inletValveCheck_->isChecked();
    settings.filamentEnabled = filamentCheck_->isChecked();
    settings.multiplierEnabled = multiplierCheck_->isChecked();
    settings.targetSourceTemperature = sourceTempSpin_->value();
    settings.targetChamberTemperature = chamberTempSpin_->value();
    return settings;
}

}  // namespace deviceapp
