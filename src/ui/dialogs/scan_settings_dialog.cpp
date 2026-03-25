#include "ui/dialogs/scan_settings_dialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

namespace deviceapp {

namespace {

constexpr double kTargetMasses[6] = {61.0, 131.0, 219.0, 264.0, 414.0, 502.0};

double sampleRateFromText(const QString &text) {
    if (text == "10K") {
        return 10000.0;
    }
    if (text == "20K") {
        return 20000.0;
    }
    if (text == "50K") {
        return 50000.0;
    }
    if (text == "100K") {
        return 100000.0;
    }
    return 20000.0;
}

QString sampleRateToText(double sampleRate) {
    if (qFuzzyCompare(sampleRate, 10000.0)) {
        return "10K";
    }
    if (qFuzzyCompare(sampleRate, 50000.0)) {
        return "50K";
    }
    if (qFuzzyCompare(sampleRate, 100000.0)) {
        return "100K";
    }
    return "20K";
}

}  // namespace

ScanSettingsDialog::ScanSettingsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("扫描设置");
    resize(840, 680);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);

    tabWidget_ = new QTabWidget(this);
    layout->addWidget(tabWidget_, 1);

    auto *parameterTab = new QWidget(this);
    auto *parameterTabLayout = new QVBoxLayout(parameterTab);
    parameterTabLayout->setContentsMargins(0, 0, 0, 0);
    parameterTabLayout->setSpacing(12);

    auto *parameterGroup = new QGroupBox("扫描参数", this);
    parameterGroup->setStyleSheet("QGroupBox { font-weight: 600; border: 1px solid #d0d5dd; margin-top: 8px; }"
                                  "QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; }");
    auto *parameterLayout = new QVBoxLayout(parameterGroup);
    parameterLayout->setContentsMargins(12, 16, 12, 12);
    parameterLayout->setSpacing(12);

    auto *modeGroup = new QGroupBox("扫描方式", this);
    auto *modeLayout = new QVBoxLayout(modeGroup);
    auto *modeSelectLayout = new QHBoxLayout();
    fullScanButton_ = new QRadioButton("全扫描", this);
    simButton_ = new QRadioButton("选择离子扫描", this);
    fullScanButton_->setChecked(true);
    modeSelectLayout->addWidget(fullScanButton_);
    modeSelectLayout->addWidget(simButton_);
    modeSelectLayout->addStretch();
    modeLayout->addLayout(modeSelectLayout);

    massAxisShortcutGroup_ = new QGroupBox("质量轴设置", this);
    auto *shortcutLayout = new QGridLayout(massAxisShortcutGroup_);
    shortcutSlopeValueLabel_ = new QLabel(this);
    shortcutOffsetValueLabel_ = new QLabel(this);
    shortcutEnabledValueLabel_ = new QLabel(this);
    openMassAxisTabButton_ = new QPushButton("切换到质量轴页", this);
    shortcutLayout->addWidget(new QLabel("K", this), 0, 0);
    shortcutLayout->addWidget(shortcutSlopeValueLabel_, 0, 1);
    shortcutLayout->addWidget(new QLabel("B", this), 0, 2);
    shortcutLayout->addWidget(shortcutOffsetValueLabel_, 0, 3);
    shortcutLayout->addWidget(new QLabel("是否使用质量轴", this), 1, 0, 1, 2);
    shortcutLayout->addWidget(shortcutEnabledValueLabel_, 1, 2);
    shortcutLayout->addWidget(openMassAxisTabButton_, 1, 3);
    modeLayout->addWidget(massAxisShortcutGroup_);
    parameterLayout->addWidget(modeGroup);

    auto *targetGroup = new QGroupBox("目标离子", this);
    auto *targetLayout = new QGridLayout(targetGroup);
    targetLayout->addWidget(new QLabel("质量数", this), 0, 0);
    auto *massRow = new QHBoxLayout();
    massRow->setSpacing(8);
    auto *voltageRow = new QHBoxLayout();
    voltageRow->setSpacing(8);
    for (int i = 0; i < 6; ++i) {
        targetIonChecks_[i] = new QCheckBox(QString::number(static_cast<int>(kTargetMasses[i])), this);
        targetVoltageSpins_[i] = new QDoubleSpinBox(this);
        targetVoltageSpins_[i]->setRange(-2000.0, 5000.0);
        targetVoltageSpins_[i]->setDecimals(1);
        targetVoltageSpins_[i]->setSuffix(" V");
        targetVoltageSpins_[i]->setFixedWidth(88);
        massRow->addWidget(targetIonChecks_[i]);
        voltageRow->addWidget(targetVoltageSpins_[i]);
    }
    massRow->addStretch();
    voltageRow->addStretch();
    auto *massRowWidget = new QWidget(this);
    massRowWidget->setLayout(massRow);
    targetLayout->addWidget(massRowWidget, 0, 1);
    targetLayout->addWidget(new QLabel("电压设置", this), 1, 0);
    auto *voltageRowWidget = new QWidget(this);
    voltageRowWidget->setLayout(voltageRow);
    targetLayout->addWidget(voltageRowWidget, 1, 1);
    parameterLayout->addWidget(targetGroup);

    fullScanGroup_ = new QGroupBox("全扫描设置", this);
    auto *fullScanLayout = new QGridLayout(fullScanGroup_);
    startSpin_ = new QDoubleSpinBox(this);
    startSpin_->setRange(1.0, 1000.0);
    startSpin_->setDecimals(2);
    endSpin_ = new QDoubleSpinBox(this);
    endSpin_->setRange(1.0, 1000.0);
    endSpin_->setDecimals(2);
    scanSpeedSpin_ = new QDoubleSpinBox(this);
    scanSpeedSpin_->setRange(1.0, 50000.0);
    scanSpeedSpin_->setDecimals(1);
    scanSpeedSpin_->setSuffix(" u/s");
    scanTimePresetBox_ = new QComboBox(this);
    scanTimePresetBox_->addItems({"100 ms", "200 ms", "500 ms", "1000 ms", "自定义"});
    customScanTimeSpin_ = new QDoubleSpinBox(this);
    customScanTimeSpin_->setRange(1.0, 10000.0);
    customScanTimeSpin_->setDecimals(1);
    customScanTimeSpin_->setSuffix(" ms");
    flybackSpin_ = new QDoubleSpinBox(this);
    flybackSpin_->setRange(0.0, 5000.0);
    flybackSpin_->setDecimals(1);
    flybackSpin_->setSuffix(" ms");
    useTargetVoltageCheck_ = new QCheckBox("是否使用电压", this);
    voltageRangeMinSpin_ = new QDoubleSpinBox(this);
    voltageRangeMinSpin_->setRange(-2000.0, 5000.0);
    voltageRangeMinSpin_->setDecimals(1);
    voltageRangeMinSpin_->setSuffix(" V");
    voltageRangeMaxSpin_ = new QDoubleSpinBox(this);
    voltageRangeMaxSpin_->setRange(-2000.0, 5000.0);
    voltageRangeMaxSpin_->setDecimals(1);
    voltageRangeMaxSpin_->setSuffix(" V");
    calculateVoltageButton_ = new QPushButton("计算", this);
    fullScanLayout->addWidget(new QLabel("质量数", this), 0, 0);
    auto *massRangeLayout = new QHBoxLayout();
    massRangeLayout->addWidget(startSpin_);
    massRangeLayout->addWidget(new QLabel("至", this));
    massRangeLayout->addWidget(endSpin_);
    auto *massRangeWidget = new QWidget(this);
    massRangeWidget->setLayout(massRangeLayout);
    fullScanLayout->addWidget(massRangeWidget, 0, 1, 1, 3);
    fullScanLayout->addWidget(new QLabel("扫描速度", this), 1, 0);
    fullScanLayout->addWidget(scanSpeedSpin_, 1, 1);
    fullScanLayout->addWidget(new QLabel("扫描时间", this), 1, 2);
    auto *scanTimeLayout = new QHBoxLayout();
    scanTimeLayout->addWidget(scanTimePresetBox_);
    scanTimeLayout->addWidget(customScanTimeSpin_);
    auto *scanTimeWidget = new QWidget(this);
    scanTimeWidget->setLayout(scanTimeLayout);
    fullScanLayout->addWidget(scanTimeWidget, 1, 3);
    fullScanLayout->addWidget(new QLabel("回扫时间", this), 2, 0);
    fullScanLayout->addWidget(flybackSpin_, 2, 1);
    fullScanLayout->addWidget(useTargetVoltageCheck_, 2, 2);
    auto *voltageLayout = new QHBoxLayout();
    voltageLayout->addWidget(voltageRangeMinSpin_);
    voltageLayout->addWidget(new QLabel("至", this));
    voltageLayout->addWidget(voltageRangeMaxSpin_);
    voltageLayout->addWidget(calculateVoltageButton_);
    auto *voltageWidget = new QWidget(this);
    voltageWidget->setLayout(voltageLayout);
    fullScanLayout->addWidget(voltageWidget, 2, 3);
    parameterLayout->addWidget(fullScanGroup_);

    selectedIonGroup_ = new QGroupBox("选择离子扫描设置", this);
    auto *selectedIonLayout = new QGridLayout(selectedIonGroup_);
    dwellTimeSpin_ = new QDoubleSpinBox(this);
    dwellTimeSpin_->setRange(0.1, 10000.0);
    dwellTimeSpin_->setDecimals(1);
    dwellTimeSpin_->setSuffix(" ms");
    selectedIonFlybackSpin_ = new QDoubleSpinBox(this);
    selectedIonFlybackSpin_->setRange(0.0, 5000.0);
    selectedIonFlybackSpin_->setDecimals(1);
    selectedIonFlybackSpin_->setSuffix(" ms");
    selectedIonPeakWidthSpin_ = new QDoubleSpinBox(this);
    selectedIonPeakWidthSpin_->setRange(0.1, 20.0);
    selectedIonPeakWidthSpin_->setDecimals(2);
    selectedIonRampVoltageSpin_ = new QDoubleSpinBox(this);
    selectedIonRampVoltageSpin_->setRange(-2000.0, 2000.0);
    selectedIonRampVoltageSpin_->setDecimals(1);
    selectedIonRampVoltageSpin_->setSuffix(" V");
    calculateSelectedIonButton_ = new QPushButton("计算", this);
    selectedIonLayout->addWidget(new QLabel("驻留时间", this), 0, 0);
    selectedIonLayout->addWidget(dwellTimeSpin_, 0, 1);
    selectedIonLayout->addWidget(new QLabel("回扫时间", this), 0, 2);
    selectedIonLayout->addWidget(selectedIonFlybackSpin_, 0, 3);
    selectedIonLayout->addWidget(new QLabel("目标峰宽度", this), 1, 0);
    selectedIonLayout->addWidget(selectedIonPeakWidthSpin_, 1, 1);
    selectedIonLayout->addWidget(new QLabel("斜扫电压", this), 1, 2);
    selectedIonLayout->addWidget(selectedIonRampVoltageSpin_, 1, 3);
    selectedIonLayout->addWidget(calculateSelectedIonButton_, 2, 3);
    parameterLayout->addWidget(selectedIonGroup_);

    auto *rfadGroup = new QGroupBox("RFAD 设置", this);
    auto *rfadLayout = new QHBoxLayout(rfadGroup);
    sampleRateBox_ = new QComboBox(this);
    sampleRateBox_->addItems({"10K", "20K", "50K", "100K"});
    rfadSettingsButton_ = new QPushButton("设置", this);
    rfadLayout->addWidget(new QLabel("采样频率", this));
    rfadLayout->addWidget(sampleRateBox_);
    rfadLayout->addSpacing(16);
    rfadLayout->addWidget(rfadSettingsButton_);
    rfadLayout->addStretch();
    parameterLayout->addWidget(rfadGroup);
    parameterTabLayout->addWidget(parameterGroup);
    tabWidget_->addTab(parameterTab, "扫描参数");

    auto *massAxisTab = new QWidget(this);
    auto *massAxisTabLayout = new QVBoxLayout(massAxisTab);
    massAxisTabLayout->setContentsMargins(0, 0, 0, 0);
    massAxisTabLayout->setSpacing(12);

    auto *axisGroup = new QGroupBox("质量轴", this);
    auto *axisLayout = new QVBoxLayout(axisGroup);
    axisLayout->setContentsMargins(12, 16, 12, 12);
    axisLayout->setSpacing(12);
    auto *axisBox = new QGroupBox("质量轴参数", this);
    auto *axisForm = new QFormLayout(axisBox);
    useMassAxisCheck_ = new QCheckBox("启用质量轴校准", this);
    massAxisSlopeSpin_ = new QDoubleSpinBox(this);
    massAxisSlopeSpin_->setRange(-100.0, 100.0);
    massAxisSlopeSpin_->setDecimals(6);
    massAxisOffsetSpin_ = new QDoubleSpinBox(this);
    massAxisOffsetSpin_->setRange(-100000.0, 100000.0);
    massAxisOffsetSpin_->setDecimals(4);
    samplingPointsSpin_ = new QSpinBox(this);
    samplingPointsSpin_->setRange(64, 1000000);
    samplingPointsSpin_->setSingleStep(64);
    applyMassAxisButton_ = new QPushButton("全部应用", this);
    axisForm->addRow("", useMassAxisCheck_);
    axisForm->addRow("质量轴 K", massAxisSlopeSpin_);
    axisForm->addRow("质量轴 B", massAxisOffsetSpin_);
    axisForm->addRow("采样点数", samplingPointsSpin_);
    axisForm->addRow("", applyMassAxisButton_);
    auto *axisHint = new QLabel("用于恢复质量轴修正参数；未启用时按默认轴计算。", this);
    axisHint->setWordWrap(true);
    axisHint->setStyleSheet("QLabel { color: #475467; background: #f8fafc; border: 1px solid #d0d5dd; padding: 8px; }");
    axisLayout->addWidget(axisBox);
    axisLayout->addWidget(axisHint);
    massAxisTabLayout->addWidget(axisGroup);
    tabWidget_->addTab(massAxisTab, "质量轴");

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);

    connect(fullScanButton_, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) {
            settings_.mode = ScanMode::FullScan;
            updateModeUi();
        }
    });
    connect(simButton_, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) {
            settings_.mode = ScanMode::SelectedIon;
            updateModeUi();
        }
    });
    connect(scanTimePresetBox_, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        customScanTimeSpin_->setEnabled(text == "自定义");
    });
    connect(useTargetVoltageCheck_, &QCheckBox::toggled, this, &ScanSettingsDialog::setVoltageInputsEnabled);
    connect(applyMassAxisButton_, &QPushButton::clicked, this, &ScanSettingsDialog::applyMassAxisShortcut);
    connect(openMassAxisTabButton_, &QPushButton::clicked, this, [this]() { tabWidget_->setCurrentIndex(1); });
    connect(calculateVoltageButton_, &QPushButton::clicked, this, &ScanSettingsDialog::calculateFullScanVoltages);
    connect(calculateSelectedIonButton_, &QPushButton::clicked, this, &ScanSettingsDialog::calculateSelectedIonValues);
    connect(rfadSettingsButton_, &QPushButton::clicked, this, &ScanSettingsDialog::openRfadSettingsDialog);
    connect(useMassAxisCheck_, &QCheckBox::toggled, this, [this]() { refreshMassAxisSummary(); });
    connect(massAxisSlopeSpin_, &QDoubleSpinBox::valueChanged, this, [this](double) { refreshMassAxisSummary(); });
    connect(massAxisOffsetSpin_, &QDoubleSpinBox::valueChanged, this, [this](double) { refreshMassAxisSummary(); });
}

void ScanSettingsDialog::setSettings(const ScanSettings &settings) {
    settings_ = settings;
    fullScanButton_->setChecked(settings.mode == ScanMode::FullScan);
    simButton_->setChecked(settings.mode == ScanMode::SelectedIon);
    startSpin_->setValue(settings.massStart);
    endSpin_->setValue(settings.massEnd);
    scanSpeedSpin_->setValue(settings.scanSpeed);
    scanTimePresetBox_->setCurrentText(settings.scanTimePreset);
    customScanTimeSpin_->setValue(settings.scanTimeMs);
    flybackSpin_->setValue(settings.flybackTimeMs);
    sampleRateBox_->setCurrentText(settings.sampleRateText.isEmpty() ? sampleRateToText(settings.sampleRateHz) : settings.sampleRateText);
    samplingPointsSpin_->setValue(settings.samplingPoints);
    dwellTimeSpin_->setValue(settings.targetDwellTimeMs);
    selectedIonFlybackSpin_->setValue(settings.flybackTimeMs);
    selectedIonPeakWidthSpin_->setValue(settings.targetPeakWidth);
    selectedIonRampVoltageSpin_->setValue(settings.rampVoltage);
    useTargetVoltageCheck_->setChecked(settings.useTargetVoltage);
    voltageRangeMinSpin_->setValue(settings.voltageRangeMin);
    voltageRangeMaxSpin_->setValue(settings.voltageRangeMax);
    useMassAxisCheck_->setChecked(settings.useMassAxisCalibration);
    massAxisSlopeSpin_->setValue(settings.massAxisSlope);
    massAxisOffsetSpin_->setValue(settings.massAxisOffset);
    setSelectedTargetIons(settings.targetIons);
    setTargetVoltages(settings.targetVoltages);
    setVoltageInputsEnabled(settings.useTargetVoltage);
    refreshMassAxisSummary();
    updateModeUi();
}

ScanSettings ScanSettingsDialog::settings() const {
    ScanSettings settings = settings_;
    settings.mode = fullScanButton_->isChecked() ? ScanMode::FullScan : ScanMode::SelectedIon;
    settings.massStart = startSpin_->value();
    settings.massEnd = endSpin_->value();
    settings.scanSpeed = scanSpeedSpin_->value();
    settings.scanTimePreset = scanTimePresetBox_->currentText();
    settings.scanTimeMs = customScanTimeSpin_->value();
    settings.flybackTimeMs = settings.mode == ScanMode::FullScan ? flybackSpin_->value() : selectedIonFlybackSpin_->value();
    settings.sampleRateText = sampleRateBox_->currentText();
    settings.sampleRateHz = sampleRateFromText(settings.sampleRateText);
    settings.samplingPoints = samplingPointsSpin_->value();
    settings.useMassAxisCalibration = useMassAxisCheck_->isChecked();
    settings.massAxisSlope = massAxisSlopeSpin_->value();
    settings.massAxisOffset = massAxisOffsetSpin_->value();
    settings.targetDwellTimeMs = dwellTimeSpin_->value();
    settings.targetPeakWidth = selectedIonPeakWidthSpin_->value();
    settings.rampVoltage = selectedIonRampVoltageSpin_->value();
    settings.useTargetVoltage = useTargetVoltageCheck_->isChecked();
    settings.voltageRangeMin = voltageRangeMinSpin_->value();
    settings.voltageRangeMax = voltageRangeMaxSpin_->value();
    settings.targetIons = selectedTargetIons();
    settings.targetVoltages = collectTargetVoltages();
    if (settings.scanTimePreset != "自定义") {
        bool ok = false;
        const double presetValue = settings.scanTimePreset.left(settings.scanTimePreset.indexOf(' ')).toDouble(&ok);
        if (ok) {
            settings.scanTimeMs = presetValue;
        }
    }
    return settings;
}

void ScanSettingsDialog::setVoltageInputsEnabled(bool enabled) {
    voltageRangeMinSpin_->setEnabled(enabled);
    voltageRangeMaxSpin_->setEnabled(enabled);
    calculateVoltageButton_->setEnabled(enabled);
    for (auto *spin : targetVoltageSpins_) {
        spin->setEnabled(enabled);
    }
}

void ScanSettingsDialog::updateModeUi() {
    const bool fullScan = fullScanButton_->isChecked();
    massAxisShortcutGroup_->setVisible(true);
    fullScanGroup_->setVisible(fullScan);
    useTargetVoltageCheck_->setEnabled(fullScan);
    setVoltageInputsEnabled(fullScan && useTargetVoltageCheck_->isChecked());
    selectedIonGroup_->setVisible(!fullScan);
}

void ScanSettingsDialog::applyMassAxisShortcut() {
    refreshMassAxisSummary();
}

void ScanSettingsDialog::calculateFullScanVoltages() {
    const QVector<double> ions = selectedTargetIons();
    if (ions.isEmpty()) {
        return;
    }
    const double minVoltage = voltageRangeMinSpin_->value();
    const double maxVoltage = voltageRangeMaxSpin_->value();
    const double span = ions.size() > 1 ? (maxVoltage - minVoltage) / static_cast<double>(ions.size() - 1) : 0.0;
    for (int i = 0; i < 6; ++i) {
        if (!targetIonChecks_[i]->isChecked()) {
            continue;
        }
        int selectedIndex = 0;
        for (int j = 0; j < i; ++j) {
            if (targetIonChecks_[j]->isChecked()) {
                ++selectedIndex;
            }
        }
        targetVoltageSpins_[i]->setValue(minVoltage + span * selectedIndex);
    }
}

void ScanSettingsDialog::calculateSelectedIonValues() {
    const int selectedCount = selectedTargetIons().size();
    if (selectedCount <= 0) {
        return;
    }
    selectedIonPeakWidthSpin_->setValue(qMax(0.1, selectedIonPeakWidthSpin_->value()));
    selectedIonRampVoltageSpin_->setValue(30.0 * selectedCount);
}

void ScanSettingsDialog::openRfadSettingsDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle("RFAD 附加设置");
    auto *layout = new QFormLayout(&dialog);
    auto *pointsSpin = new QSpinBox(&dialog);
    pointsSpin->setRange(64, 1000000);
    pointsSpin->setSingleStep(64);
    pointsSpin->setValue(samplingPointsSpin_->value());
    auto *peakWidthSpin = new QDoubleSpinBox(&dialog);
    peakWidthSpin->setRange(0.1, 20.0);
    peakWidthSpin->setDecimals(2);
    peakWidthSpin->setValue(selectedIonPeakWidthSpin_->value());
    auto *rampSpin = new QDoubleSpinBox(&dialog);
    rampSpin->setRange(-2000.0, 2000.0);
    rampSpin->setDecimals(1);
    rampSpin->setValue(selectedIonRampVoltageSpin_->value());
    layout->addRow("采样点数", pointsSpin);
    layout->addRow("峰宽", peakWidthSpin);
    layout->addRow("斜坡电压", rampSpin);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    samplingPointsSpin_->setValue(pointsSpin->value());
    selectedIonPeakWidthSpin_->setValue(peakWidthSpin->value());
    selectedIonRampVoltageSpin_->setValue(rampSpin->value());
}

QVector<double> ScanSettingsDialog::selectedTargetIons() const {
    QVector<double> values;
    for (int i = 0; i < 6; ++i) {
        if (targetIonChecks_[i]->isChecked()) {
            values.append(kTargetMasses[i]);
        }
    }
    return values;
}

QVector<double> ScanSettingsDialog::collectTargetVoltages() const {
    QVector<double> values;
    for (int i = 0; i < 6; ++i) {
        if (targetIonChecks_[i]->isChecked()) {
            values.append(targetVoltageSpins_[i]->value());
        }
    }
    return values;
}

void ScanSettingsDialog::setSelectedTargetIons(const QVector<double> &ions) {
    for (int i = 0; i < 6; ++i) {
        bool checked = false;
        for (double ion : ions) {
            if (qAbs(ion - kTargetMasses[i]) < 0.1) {
                checked = true;
                break;
            }
        }
        targetIonChecks_[i]->setChecked(checked);
    }
}

void ScanSettingsDialog::setTargetVoltages(const QVector<double> &voltages) {
    int index = 0;
    for (int i = 0; i < 6; ++i) {
        if (targetIonChecks_[i]->isChecked() && index < voltages.size()) {
            targetVoltageSpins_[i]->setValue(voltages[index++]);
        } else {
            targetVoltageSpins_[i]->setValue(0.0);
        }
    }
}

void ScanSettingsDialog::refreshMassAxisSummary() {
    shortcutSlopeValueLabel_->setText(QString::number(massAxisSlopeSpin_->value(), 'f', 6));
    shortcutOffsetValueLabel_->setText(QString::number(massAxisOffsetSpin_->value(), 'f', 4));
    shortcutEnabledValueLabel_->setText(useMassAxisCheck_->isChecked() ? "是" : "否");
}

}  // namespace deviceapp
