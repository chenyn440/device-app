#include "ui/dialogs/chart_settings_dialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace deviceapp {

ChartSettingsDialog::ChartSettingsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("谱图设置");
    resize(900, 420);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(12);

    auto *contentRow = new QHBoxLayout();
    contentRow->setSpacing(12);

    auto *xGroup = new QGroupBox("X轴显示", this);
    auto *xLayout = new QVBoxLayout(xGroup);
    xAxisMassRadio_ = new QRadioButton("质量数", this);
    xAxisVoltageRadio_ = new QRadioButton("电压", this);
    xAxisTimeRadio_ = new QRadioButton("时间", this);
    xAxisPointRadio_ = new QRadioButton("点数", this);
    xLayout->addWidget(xAxisMassRadio_);
    xLayout->addWidget(xAxisVoltageRadio_);
    xLayout->addWidget(xAxisTimeRadio_);
    xLayout->addWidget(xAxisPointRadio_);
    xLayout->addStretch();

    auto *yGroup = new QGroupBox("Y轴显示", this);
    auto *yLayout = new QGridLayout(yGroup);
    disableAutoRestoreCheck_ = new QCheckBox("谱图不自动恢复全屏", this);
    fixedYRangeCheck_ = new QCheckBox("Y轴范围固定", this);
    yMinSpin_ = new QDoubleSpinBox(this);
    yMinSpin_->setRange(-1000000.0, 1000000.0);
    yMinSpin_->setDecimals(2);
    yMaxSpin_ = new QDoubleSpinBox(this);
    yMaxSpin_->setRange(-1000000.0, 1000000.0);
    yMaxSpin_->setDecimals(2);
    yLayout->addWidget(disableAutoRestoreCheck_, 0, 0, 1, 2);
    yLayout->addWidget(fixedYRangeCheck_, 1, 0, 1, 2);
    yLayout->addWidget(new QLabel("最小值", this), 2, 0);
    yLayout->addWidget(yMinSpin_, 2, 1);
    yLayout->addWidget(new QLabel("最大值", this), 3, 0);
    yLayout->addWidget(yMaxSpin_, 3, 1);
    yLayout->setRowStretch(4, 1);

    auto *dataGroup = new QGroupBox("数据显示", this);
    auto *dataLayout = new QGridLayout(dataGroup);
    peakCountSpin_ = new QSpinBox(this);
    peakCountSpin_->setRange(0, 999);
    showRawDataCheck_ = new QCheckBox("显示原始数据", this);
    showPersistenceCheck_ = new QCheckBox("显示余晖", this);
    showHalfPeakWidthCheck_ = new QCheckBox("显示半峰宽", this);
    dataLayout->addWidget(new QLabel("谱峰个数", this), 0, 0);
    dataLayout->addWidget(peakCountSpin_, 0, 1);
    dataLayout->addWidget(showRawDataCheck_, 1, 0, 1, 2);
    dataLayout->addWidget(showPersistenceCheck_, 2, 0, 1, 2);
    dataLayout->addWidget(showHalfPeakWidthCheck_, 3, 0, 1, 2);
    dataLayout->setRowStretch(4, 1);

    contentRow->addWidget(xGroup, 1);
    contentRow->addWidget(yGroup, 2);
    contentRow->addWidget(dataGroup, 2);
    layout->addLayout(contentRow);

    auto *bottomRow = new QHBoxLayout();
    showTicCheck_ = new QCheckBox("显示TIC谱图", this);
    bottomRow->addWidget(showTicCheck_);
    bottomRow->addStretch();
    layout->addLayout(bottomRow);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
    connect(fixedYRangeCheck_, &QCheckBox::toggled, this, [this]() { updateYRangeState(); });
}

void ChartSettingsDialog::setSettings(const ChartSettings &settings) {
    xAxisMassRadio_->setChecked(settings.xAxisMode == "mass");
    xAxisVoltageRadio_->setChecked(settings.xAxisMode == "voltage");
    xAxisTimeRadio_->setChecked(settings.xAxisMode == "time");
    xAxisPointRadio_->setChecked(settings.xAxisMode == "point");
    if (!xAxisMassRadio_->isChecked() && !xAxisVoltageRadio_->isChecked()
        && !xAxisTimeRadio_->isChecked() && !xAxisPointRadio_->isChecked()) {
        xAxisMassRadio_->setChecked(true);
    }
    disableAutoRestoreCheck_->setChecked(settings.disableAutoRestoreFullScale);
    fixedYRangeCheck_->setChecked(settings.fixedYRange);
    yMinSpin_->setValue(settings.yMin);
    yMaxSpin_->setValue(settings.yMax);
    peakCountSpin_->setValue(settings.peakCount);
    showRawDataCheck_->setChecked(settings.showRawData);
    showPersistenceCheck_->setChecked(settings.showPersistence);
    showHalfPeakWidthCheck_->setChecked(settings.showHalfPeakWidth);
    showTicCheck_->setChecked(settings.showTicChart);
    updateYRangeState();
}

ChartSettings ChartSettingsDialog::settings() const {
    ChartSettings settings;
    settings.xAxisMode = xAxisVoltageRadio_->isChecked() ? "voltage"
        : xAxisTimeRadio_->isChecked() ? "time"
        : xAxisPointRadio_->isChecked() ? "point"
        : "mass";
    settings.disableAutoRestoreFullScale = disableAutoRestoreCheck_->isChecked();
    settings.fixedYRange = fixedYRangeCheck_->isChecked();
    settings.yMin = yMinSpin_->value();
    settings.yMax = yMaxSpin_->value();
    settings.peakCount = peakCountSpin_->value();
    settings.showRawData = showRawDataCheck_->isChecked();
    settings.showPersistence = showPersistenceCheck_->isChecked();
    settings.showHalfPeakWidth = showHalfPeakWidthCheck_->isChecked();
    settings.showTicChart = showTicCheck_->isChecked();
    return settings;
}

void ChartSettingsDialog::updateYRangeState() {
    const bool enabled = fixedYRangeCheck_->isChecked();
    yMinSpin_->setEnabled(enabled);
    yMaxSpin_->setEnabled(enabled);
}

}  // namespace deviceapp
