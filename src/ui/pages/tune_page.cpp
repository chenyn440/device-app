#include "ui/pages/tune_page.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace deviceapp {

namespace {

QVector<double> parseTargetIons(const QString &text) {
    QVector<double> values;
    for (const QString &part : text.split(",", Qt::SkipEmptyParts)) {
        values.append(part.trimmed().toDouble());
    }
    return values;
}

const QString kTopPanelFrameStyle = "QFrame { background: #f6f7f9; border: 1px solid #cfd4dc; }";
const QString kTopGroupStyle =
    "QFrame { border: 1px solid #d8dbe2; background: #ffffff; }"
    "QCheckBox { color: #344054; spacing: 4px; font-size: 12px; }";
const QString kTopFieldLabelStyle = "QLabel { color: #475467; background: transparent; border: none; font-size: 12px; font-weight: 600; }";
const QString kTopSpinStyle =
    "QDoubleSpinBox { min-height: 20px; padding: 0 4px; border: 1px solid #cfd4dc; border-radius: 3px; background: #ffffff; color: #344054; }";
const QString kTopButtonStyle =
    "QPushButton { min-height: 22px; padding: 0 12px; background: #f5f6f8; border: 1px solid #cfd4dc; border-radius: 4px; color: #475467; font-size: 12px; font-weight: 600; }"
    "QPushButton:hover { background: #eef1f5; }"
    "QPushButton:pressed { background: #e4e8ee; }";

}  // namespace

TunePage::TunePage(ScanControlService *scanControlService, TuneService *tuneService, QWidget *parent)
    : QWidget(parent), scanControlService_(scanControlService), tuneService_(tuneService) {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    actionHeader_ = new PageActionHeader(this);
    mainLayout->addWidget(actionHeader_);

    scanModeBox_ = new QComboBox(this);
    scanModeBox_->addItems({"Full Scan", "SIM"});
    scanModeBox_->hide();
    detectorBox_ = new QComboBox(this);
    detectorBox_->addItems({"倍增器", "法拉第筒"});
    detectorBox_->hide();

    modeHintLabel_ = new QLabel("当前未扫描，可编辑扫描设置和调谐参数。", this);
    modeHintLabel_->hide();

    auto *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(8);
    mainLayout->addLayout(contentLayout, 1);

    auto *mainDisplayPanel = new QFrame(this);
    mainDisplayPanel->setFrameShape(QFrame::StyledPanel);
    mainDisplayPanel->setStyleSheet("QFrame { background: white; border: 1px solid #d0d5dd; }");
    auto *mainDisplayLayout = new QVBoxLayout(mainDisplayPanel);
    mainDisplayLayout->setContentsMargins(0, 0, 0, 0);
    mainDisplayLayout->setSpacing(8);

    fullScanCheck_ = new QCheckBox("Full Scan", this);
    fullScanCheck_->setChecked(true);
    simCheck_ = new QCheckBox("SIM", this);
    scanSettingsButton_ = new QPushButton("设置", this);
    scanSettingsButton_->setStyleSheet(kTopButtonStyle);
    massStartSpin_ = new QDoubleSpinBox(this);
    massStartSpin_->setRange(1.0, 1000.0);
    massStartSpin_->setValue(10.0);
    massStartSpin_->setStyleSheet(kTopSpinStyle);
    massEndSpin_ = new QDoubleSpinBox(this);
    massEndSpin_->setRange(1.0, 1000.0);
    massEndSpin_->setValue(110.0);
    massEndSpin_->setStyleSheet(kTopSpinStyle);
    resolutionSpin_ = new QDoubleSpinBox(this);
    resolutionSpin_->setRange(0.1, 10.0);
    resolutionSpin_->setValue(1.0);
    resolutionSpin_->hide();
    scanTimeSpin_ = new QDoubleSpinBox(this);
    scanTimeSpin_->setRange(0.1, 10.0);
    scanTimeSpin_->setValue(2.0);
    scanTimeSpin_->hide();
    sampleRateSpin_ = new QDoubleSpinBox(this);
    sampleRateSpin_->setRange(10.0, 5000.0);
    sampleRateSpin_->setValue(1000.0);
    sampleRateSpin_->hide();
    targetIonsEdit_ = new QLineEdit("28,44,69", this);
    targetIonsEdit_->hide();

    detectorMultiplierCheck_ = new QCheckBox("倍增器", this);
    detectorMultiplierCheck_->setChecked(true);
    detectorFaradayCheck_ = new QCheckBox("法拉第筒", this);
    stableZoneOneCheck_ = new QCheckBox("稳定区一", this);
    stableZoneOneCheck_->setChecked(true);
    stableZoneTwoCheck_ = new QCheckBox("稳定区二", this);

    targetIonBox_ = new QWidget(this);
    auto *targetIonLayout = new QGridLayout(targetIonBox_);
    const QList<int> targetCandidates = {69, 131, 219, 264, 414, 502};
    for (int i = 0; i < targetCandidates.size(); ++i) {
        auto *check = new QCheckBox(QString::number(targetCandidates[i]), this);
        if (i < 3) {
            check->setChecked(true);
        }
        targetIonChecks_.append(check);
        connect(check, &QCheckBox::toggled, this, [this](bool) { syncTargetIonSelection(); });
        check->hide();
    }
    summaryTable_ = new QTableWidget(3, 4, this);
    summaryTable_->setHorizontalHeaderLabels({"m/z", "69", "131", "219"});
    summaryTable_->verticalHeader()->setVisible(false);
    summaryTable_->horizontalHeader()->setStretchLastSection(true);
    summaryTable_->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    summaryTable_->horizontalHeader()->setMinimumHeight(22);
    summaryTable_->verticalHeader()->setDefaultSectionSize(22);
    summaryTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    summaryTable_->setSelectionMode(QAbstractItemView::NoSelection);
    summaryTable_->setFocusPolicy(Qt::NoFocus);
    summaryTable_->setStyleSheet(
        "QTableWidget { border: 1px solid #d8dbe2; gridline-color: #e4e7ec; background: #ffffff; color: #475467; }"
        "QHeaderView::section { background: #f8fafc; border: 1px solid #d8dbe2; padding: 2px 4px; color: #344054; font-weight: 600; }"
        "QTableWidget::item { padding: 2px 4px; }");
    const QStringList summaryRowLabels = {"强度[v]", "丰度比[%]", "半峰宽[m.z]"};
    for (int row = 0; row < summaryRowLabels.size(); ++row) {
        summaryTable_->setItem(row, 0, new QTableWidgetItem(summaryRowLabels[row]));
        for (int col = 1; col < summaryTable_->columnCount(); ++col) {
            summaryTable_->setItem(row, col, new QTableWidgetItem(""));
        }
    }

    auto *topSettingsFrame = new QFrame(this);
    topSettingsFrame->setStyleSheet(kTopPanelFrameStyle);
    topSettingsFrame->setMaximumHeight(124);
    topSettingsFrame->setMinimumHeight(124);
    auto *topSettingsLayout = new QHBoxLayout(topSettingsFrame);
    topSettingsLayout->setContentsMargins(6, 2, 6, 2);
    topSettingsLayout->setSpacing(6);

    auto *scanGroup = new QFrame(this);
    scanGroup->setStyleSheet(kTopGroupStyle);
    auto *scanGroupLayout = new QVBoxLayout(scanGroup);
    scanGroupLayout->setContentsMargins(8, 3, 8, 4);
    scanGroupLayout->setSpacing(3);
    auto *scanModeRow = new QHBoxLayout();
    scanModeRow->setContentsMargins(0, 0, 0, 0);
    scanModeRow->setSpacing(8);
    scanModeRow->addWidget(fullScanCheck_);
    scanModeRow->addWidget(simCheck_);
    scanModeRow->addStretch();
    scanModeRow->addWidget(scanSettingsButton_);
    scanGroupLayout->addLayout(scanModeRow);

    auto *massForm = new QGridLayout();
    massForm->setContentsMargins(0, 0, 0, 0);
    massForm->setHorizontalSpacing(6);
    massForm->setVerticalSpacing(3);
    auto *massStartLabel = new QLabel("起始质量数：", this);
    auto *massEndLabel = new QLabel("终止质量数：", this);
    massStartLabel->setStyleSheet(kTopFieldLabelStyle);
    massEndLabel->setStyleSheet(kTopFieldLabelStyle);
    massStartLabel->setFixedWidth(92);
    massEndLabel->setFixedWidth(92);
    massForm->addWidget(massStartLabel, 0, 0);
    massForm->addWidget(massStartSpin_, 0, 1);
    massForm->addWidget(massEndLabel, 1, 0);
    massForm->addWidget(massEndSpin_, 1, 1);
    scanGroupLayout->addLayout(massForm);

    auto *detectorGroup = new QFrame(this);
    detectorGroup->setStyleSheet(kTopGroupStyle);
    auto *detectorGroupLayout = new QVBoxLayout(detectorGroup);
    detectorGroupLayout->setContentsMargins(8, 3, 8, 4);
    detectorGroupLayout->setSpacing(3);
    auto *detectorTitle = new QLabel("检测器", this);
    detectorTitle->setStyleSheet(kTopFieldLabelStyle);
    auto *detectorRow = new QHBoxLayout();
    detectorRow->setContentsMargins(0, 0, 0, 0);
    detectorRow->setSpacing(8);
    detectorRow->addWidget(detectorMultiplierCheck_);
    detectorRow->addWidget(detectorFaradayCheck_);
    detectorRow->addStretch();
    auto *stableTitle = new QLabel("稳定区", this);
    stableTitle->setStyleSheet(kTopFieldLabelStyle);
    auto *stableRow = new QHBoxLayout();
    stableRow->setContentsMargins(0, 0, 0, 0);
    stableRow->setSpacing(8);
    stableRow->addWidget(stableZoneOneCheck_);
    stableRow->addWidget(stableZoneTwoCheck_);
    stableRow->addStretch();
    detectorGroupLayout->addWidget(detectorTitle);
    detectorGroupLayout->addLayout(detectorRow);
    detectorGroupLayout->addWidget(stableTitle);
    detectorGroupLayout->addLayout(stableRow);

    auto *summaryGroup = new QFrame(this);
    summaryGroup->setStyleSheet("QFrame { background: #ffffff; border: 1px solid #d8dbe2; }");
    auto *summaryLayout = new QVBoxLayout(summaryGroup);
    summaryLayout->setContentsMargins(0, 0, 0, 0);
    summaryLayout->addWidget(summaryTable_);

    topSettingsLayout->addWidget(scanGroup, 3);
    topSettingsLayout->addWidget(detectorGroup, 3);
    topSettingsLayout->addWidget(summaryGroup, 6);

    dualSpectrumPanel_ = new DualSpectrumPanel(this);
    mainDisplayLayout->addWidget(topSettingsFrame);
    mainDisplayLayout->addWidget(dualSpectrumPanel_, 1);

    auto *rightPanel = new QFrame(this);
    rightPanel->setFrameShape(QFrame::StyledPanel);
    rightPanel->setMinimumWidth(340);
    rightPanel->setStyleSheet("QFrame { background: #fcfcfd; border: 1px solid #d0d5dd; }");
    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(8, 8, 8, 8);
    rightLayout->setSpacing(8);
    const TuneParameters parameters = tuneService_->currentParameters();

    rightTabWidget_ = new QTabWidget(this);
    rightTabWidget_->setCurrentIndex(0);
    rightLayout->addWidget(rightTabWidget_);

    auto *parameterTab = new QWidget(this);
    auto *parameterTabLayout = new QVBoxLayout(parameterTab);
    parameterTabLayout->setContentsMargins(6, 6, 6, 6);
    parameterTabLayout->setSpacing(6);

    const QString groupStyle =
        "QGroupBox { font-weight: 600; border: 1px solid #d9d9d9; margin-top: 8px; background: #ffffff; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; color: #444444; }";

    showAdvancedParametersCheck_ = new QCheckBox("显示高级参数", this);
    showAdvancedParametersCheck_->setChecked(false);
    parameterTabLayout->addWidget(showAdvancedParametersCheck_);

    auto buildParameterRow = [this](const QString &label, double minimum, double maximum, double value, int decimals = 1, double step = 0.1) {
        auto *row = new ParameterSliderRow(label, "", this);
        row->setRange(minimum, maximum);
        row->setDecimals(decimals);
        row->setSingleStep(step);
        row->setValue(value);
        row->setLabelWidth(118);
        row->setCompact(true);
        return row;
    };

    auto *sourceBox = new QGroupBox("离子源", this);
    sourceBox->setStyleSheet(groupStyle);
    auto *sourceLayout = new QVBoxLayout(sourceBox);
    sourceLayout->setContentsMargins(8, 10, 8, 8);
    sourceLayout->setSpacing(4);
    repellerVoltageRow_ = buildParameterRow("推斥极 [V]", -200.0, 200.0, parameters.repellerVoltage);
    lens1VoltageRow_ = buildParameterRow("透镜1(选出) [V]", -200.0, 200.0, parameters.lens1Voltage);
    lens2VoltageRow_ = buildParameterRow("透镜2(聚焦) [V]", -200.0, 200.0, parameters.lens2Voltage);
    sourceLayout->addWidget(repellerVoltageRow_);
    sourceLayout->addWidget(lens1VoltageRow_);
    sourceLayout->addWidget(lens2VoltageRow_);

    auto *detectorBoxGroup = new QGroupBox("检测器", this);
    detectorBoxGroup->setStyleSheet(groupStyle);
    auto *detectorLayout = new QVBoxLayout(detectorBoxGroup);
    detectorLayout->setContentsMargins(8, 10, 8, 8);
    detectorLayout->setSpacing(4);
    highMassCompensationRow_ = buildParameterRow("高端补偿", -100.0, 100.0, parameters.highMassCompensation);
    lowMassCompensationRow_ = buildParameterRow("低端补偿", -100.0, 100.0, parameters.lowMassCompensation);
    multiplierVoltageRow_ = buildParameterRow("倍增器 [V]", 0.0, 3000.0, parameters.multiplierVoltage, 0, 1.0);
    detectorLayout->addWidget(highMassCompensationRow_);
    detectorLayout->addWidget(lowMassCompensationRow_);
    detectorLayout->addWidget(multiplierVoltageRow_);

    auto *otherBox = new QGroupBox("其他", this);
    otherBox->setStyleSheet(groupStyle);
    auto *otherLayout = new QVBoxLayout(otherBox);
    otherLayout->setContentsMargins(8, 10, 8, 8);
    otherLayout->setSpacing(4);
    rodVoltageRow_ = buildParameterRow("ROD [V]", -200.0, 200.0, parameters.rodVoltage);
    eVoltageRow_ = buildParameterRow("E [V]", -200.0, 200.0, parameters.eVoltage);
    electronEnergyRow_ = buildParameterRow("电子能量 [eV]", 0.0, 200.0, parameters.electronEnergy);
    filamentCurrentRow_ = buildParameterRow("灯丝电流 [uA]", 0.0, 200.0, parameters.filamentCurrent);
    outerDeflectionLensVoltageRow_ = buildParameterRow("外偏转透镜 [V]", -200.0, 200.0, parameters.outerDeflectionLensVoltage);
    innerDeflectionLensVoltageRow_ = buildParameterRow("内偏转透镜 [V]", -200.0, 200.0, parameters.innerDeflectionLensVoltage);
    preQuadrupoleFrontVoltageRow_ = buildParameterRow("前预四极 [V]", -200.0, 200.0, parameters.preQuadrupoleFrontVoltage);
    preQuadrupoleRearVoltageRow_ = buildParameterRow("后预四极 [V]", -200.0, 200.0, parameters.preQuadrupoleRearVoltage);
    otherLayout->addWidget(rodVoltageRow_);
    otherLayout->addWidget(eVoltageRow_);
    otherLayout->addWidget(electronEnergyRow_);
    otherLayout->addWidget(filamentCurrentRow_);
    otherLayout->addWidget(outerDeflectionLensVoltageRow_);
    otherLayout->addWidget(innerDeflectionLensVoltageRow_);
    otherLayout->addWidget(preQuadrupoleFrontVoltageRow_);
    otherLayout->addWidget(preQuadrupoleRearVoltageRow_);

    advancedParameterContainer_ = new QWidget(this);
    auto *advancedLayout = new QVBoxLayout(advancedParameterContainer_);
    advancedLayout->setContentsMargins(0, 0, 0, 0);
    advancedLayout->setSpacing(8);
    advancedLayout->addWidget(otherBox);

    applyParametersButton_ = new QPushButton("应用", this);
    applyParametersButton_->setFixedSize(72, 28);
    applyParametersButton_->setStyleSheet(
        "QPushButton { background: #efefef; color: #333333; border: 1px solid #bdbdbd; border-radius: 3px; }"
        "QPushButton:disabled { background: #f4f4f4; color: #a0a0a0; border-color: #d0d0d0; }");

    parameterTabLayout->addWidget(sourceBox);
    parameterTabLayout->addWidget(detectorBoxGroup);
    parameterTabLayout->addWidget(advancedParameterContainer_);
    parameterTabLayout->addWidget(applyParametersButton_, 0, Qt::AlignLeft);
    parameterTabLayout->addStretch();

    statusPanel_ = new InstrumentStatusPanel(this);

    auto *connectionTab = new QWidget(this);
    auto *connectionTabLayout = new QVBoxLayout(connectionTab);
    connectionTabLayout->setContentsMargins(10, 12, 10, 10);
    connectionTabLayout->setSpacing(10);
    auto *connectionForm = new QFormLayout();
    connectionForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    connectionForm->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    connectionForm->setHorizontalSpacing(10);
    connectionForm->setVerticalSpacing(10);
    connectionIpValueLabel_ = new QLabel("127.0.0.1", this);
    connectionPortValueLabel_ = new QLabel("9000", this);
    connectionIpValueLabel_->hide();
    connectionPortValueLabel_->hide();
    auto *ipEdit = new QLineEdit(this);
    auto *portEdit = new QLineEdit(this);
    ipEdit->setText(connectionIpValueLabel_->text());
    portEdit->setText(connectionPortValueLabel_->text());
    ipEdit->setObjectName("tuneConnectionIpEdit");
    portEdit->setObjectName("tuneConnectionPortEdit");
    ipEdit->setFixedWidth(116);
    portEdit->setFixedWidth(116);
    connectionForm->addRow("IP地址", ipEdit);
    connectionForm->addRow("端口号", portEdit);
    connectionTabLayout->addLayout(connectionForm);
    auto *buttonRow = new QHBoxLayout();
    buttonRow->setSpacing(8);
    connectDeviceButton_ = new QPushButton("建立连接", this);
    disconnectDeviceButton_ = new QPushButton("断开连接", this);
    connectDeviceButton_->setFixedSize(72, 28);
    disconnectDeviceButton_->setFixedSize(72, 28);
    connectDeviceButton_->setStyleSheet("QPushButton { background: #efefef; border: 1px solid #bdbdbd; }");
    disconnectDeviceButton_->setStyleSheet("QPushButton { background: #efefef; border: 1px solid #bdbdbd; }");
    buttonRow->addWidget(connectDeviceButton_);
    buttonRow->addWidget(disconnectDeviceButton_);
    buttonRow->addStretch();
    connectionStateValueLabel_ = new QLabel("未连接", this);
    connectionTabLayout->addLayout(buttonRow);
    connectionTabLayout->addWidget(connectionStateValueLabel_, 0, Qt::AlignLeft);
    connectionTabLayout->addStretch();

    rightTabWidget_->addTab(parameterTab, "参数配置");
    rightTabWidget_->addTab(statusPanel_, "仪器状态");
    rightTabWidget_->addTab(connectionTab, "设备连接");

    contentLayout->addWidget(mainDisplayPanel, 8);
    contentLayout->addWidget(rightPanel, 3);

    connect(actionHeader_, &PageActionHeader::startRequested, this, [this]() {
        emit startRequested(scanSettings(), tuneParameters());
    });
    connect(actionHeader_, &PageActionHeader::stopRequested, this, &TunePage::stopRequested);
    connect(actionHeader_, &PageActionHeader::saveRequested, this, &TunePage::saveRequested);
    connect(actionHeader_, &PageActionHeader::refreshRequested, this, &TunePage::refreshRequested);
    connect(actionHeader_, &PageActionHeader::calibrateRequested, this, &TunePage::calibrationRequested);
    connect(actionHeader_, &PageActionHeader::filamentSwitchRequested, this, &TunePage::filamentSwitchRequested);
    connect(actionHeader_, &PageActionHeader::switchStateChanged, this, &TunePage::switchStateChanged);
    connect(scanSettingsButton_, &QPushButton::clicked, this, &TunePage::scanSettingsRequested);
    connect(fullScanCheck_, &QCheckBox::toggled, this, [this](bool checked) {
        if (!checked) {
            return;
        }
        simCheck_->blockSignals(true);
        simCheck_->setChecked(false);
        simCheck_->blockSignals(false);
        scanModeBox_->setCurrentIndex(0);
    });
    connect(simCheck_, &QCheckBox::toggled, this, [this](bool checked) {
        if (!checked) {
            return;
        }
        fullScanCheck_->blockSignals(true);
        fullScanCheck_->setChecked(false);
        fullScanCheck_->blockSignals(false);
        scanModeBox_->setCurrentIndex(1);
    });
    connect(scanModeBox_, &QComboBox::currentIndexChanged, this, [this](int index) {
        const bool fullScan = index == 0;
        fullScanCheck_->blockSignals(true);
        simCheck_->blockSignals(true);
        fullScanCheck_->setChecked(fullScan);
        simCheck_->setChecked(!fullScan);
        fullScanCheck_->blockSignals(false);
        simCheck_->blockSignals(false);
        syncTargetIonSelection();
    });
    connect(detectorMultiplierCheck_, &QCheckBox::toggled, this, [this](bool checked) {
        if (!checked) {
            return;
        }
        detectorFaradayCheck_->blockSignals(true);
        detectorFaradayCheck_->setChecked(false);
        detectorFaradayCheck_->blockSignals(false);
        detectorBox_->setCurrentIndex(0);
    });
    connect(detectorFaradayCheck_, &QCheckBox::toggled, this, [this](bool checked) {
        if (!checked) {
            return;
        }
        detectorMultiplierCheck_->blockSignals(true);
        detectorMultiplierCheck_->setChecked(false);
        detectorMultiplierCheck_->blockSignals(false);
        detectorBox_->setCurrentIndex(1);
    });
    connect(detectorBox_, &QComboBox::currentIndexChanged, this, [this](int index) {
        const bool multiplier = index == 0;
        detectorMultiplierCheck_->blockSignals(true);
        detectorFaradayCheck_->blockSignals(true);
        detectorMultiplierCheck_->setChecked(multiplier);
        detectorFaradayCheck_->setChecked(!multiplier);
        detectorMultiplierCheck_->blockSignals(false);
        detectorFaradayCheck_->blockSignals(false);
    });
    connect(showAdvancedParametersCheck_, &QCheckBox::toggled, advancedParameterContainer_, &QWidget::setVisible);
    connect(applyParametersButton_, &QPushButton::clicked, this, [this]() {
        tuneService_->applyTuneParameters(tuneParameters());
        modeHintLabel_->setText("调谐参数已应用到当前设备。");
    });
    connect(connectDeviceButton_, &QPushButton::clicked, this, [this, ipEdit, portEdit]() {
        DeviceConnectionConfig config;
        config.host = ipEdit->text().trimmed();
        config.port = static_cast<quint16>(portEdit->text().trimmed().toUShort());
        emit connectRequested(config);
    });
    connect(disconnectDeviceButton_, &QPushButton::clicked, this, &TunePage::disconnectRequested);
    connect(stableZoneOneCheck_, &QCheckBox::toggled, this, [this](bool checked) {
        if (!checked) {
            return;
        }
        stableZoneTwoCheck_->blockSignals(true);
        stableZoneTwoCheck_->setChecked(false);
        stableZoneTwoCheck_->blockSignals(false);
    });
    connect(stableZoneTwoCheck_, &QCheckBox::toggled, this, [this](bool checked) {
        if (!checked) {
            return;
        }
        stableZoneOneCheck_->blockSignals(true);
        stableZoneOneCheck_->setChecked(false);
        stableZoneOneCheck_->blockSignals(false);
    });
    syncTargetIonSelection();
    advancedParameterContainer_->setVisible(showAdvancedParametersCheck_->isChecked());
    scanSettingsCache_.mode = ScanMode::FullScan;
    scanSettingsCache_.massStart = massStartSpin_->value();
    scanSettingsCache_.massEnd = massEndSpin_->value();
    scanSettingsCache_.scanTimeMs = scanTimeSpin_->value() * 1000.0;
    scanSettingsCache_.sampleRateHz = sampleRateSpin_->value();
    scanSettingsCache_.targetIons = parseTargetIons(targetIonsEdit_->text());
    setScanState(false, false);
}

ScanSettings TunePage::scanSettings() const {
    ScanSettings settings = scanSettingsCache_;
    settings.mode = scanModeBox_->currentIndex() == 0 ? ScanMode::FullScan : ScanMode::SelectedIon;
    settings.massStart = massStartSpin_->value();
    settings.massEnd = massEndSpin_->value();
    settings.scanTimeMs = scanTimeSpin_->value() * 1000.0;
    settings.sampleRateHz = sampleRateSpin_->value();
    settings.targetIons = parseTargetIons(targetIonsEdit_->text());
    if (!settings.useTargetVoltage) {
        settings.targetVoltages.clear();
    }
    return settings;
}

void TunePage::applyScanSettings(const ScanSettings &settings) {
    scanSettingsCache_ = settings;

    scanModeBox_->setCurrentIndex(settings.mode == ScanMode::FullScan ? 0 : 1);
    massStartSpin_->setValue(settings.massStart);
    massEndSpin_->setValue(settings.massEnd);
    scanTimeSpin_->setValue(settings.scanTimeMs / 1000.0);
    sampleRateSpin_->setValue(settings.sampleRateHz);

    QStringList ions;
    for (double ion : settings.targetIons) {
        ions << QString::number(ion, 'f', ion == qRound64(ion) ? 0 : 2);
    }
    targetIonsEdit_->setText(ions.join(","));

    for (auto *check : targetIonChecks_) {
        const double targetValue = check->text().toDouble();
        const bool checked = std::any_of(settings.targetIons.cbegin(), settings.targetIons.cend(), [targetValue](double ion) {
            return qAbs(ion - targetValue) < 0.1;
        });
        check->blockSignals(true);
        check->setChecked(checked);
        check->blockSignals(false);
    }
    fullScanCheck_->setChecked(settings.mode == ScanMode::FullScan);
    simCheck_->setChecked(settings.mode == ScanMode::SelectedIon);
    syncTargetIonSelection();
}

TuneParameters TunePage::tuneParameters() const {
    TuneParameters parameters;
    parameters.detector = detectorBox_->currentIndex() == 0 ? DetectorType::ElectronMultiplier : DetectorType::FaradayCup;
    parameters.repellerVoltage = repellerVoltageRow_->value();
    parameters.lens1Voltage = lens1VoltageRow_->value();
    parameters.lens2Voltage = lens2VoltageRow_->value();
    parameters.highMassCompensation = highMassCompensationRow_->value();
    parameters.lowMassCompensation = lowMassCompensationRow_->value();
    parameters.multiplierVoltage = multiplierVoltageRow_->value();
    parameters.rodVoltage = rodVoltageRow_->value();
    parameters.eVoltage = eVoltageRow_->value();
    parameters.electronEnergy = electronEnergyRow_->value();
    parameters.filamentCurrent = filamentCurrentRow_->value();
    parameters.outerDeflectionLensVoltage = outerDeflectionLensVoltageRow_->value();
    parameters.innerDeflectionLensVoltage = innerDeflectionLensVoltageRow_->value();
    parameters.preQuadrupoleFrontVoltage = preQuadrupoleFrontVoltageRow_->value();
    parameters.preQuadrupoleRearVoltage = preQuadrupoleRearVoltageRow_->value();
    return parameters;
}

void TunePage::setInstrumentStatus(const InstrumentStatus &status) {
    for (auto it = status.switchStates.cbegin(); it != status.switchStates.cend(); ++it) {
        actionHeader_->setSwitchState(it.key(), it.value());
    }
    connectionStateValueLabel_->setText(status.connected ? "已连接" : "未连接");

    const TuneParameters parameters = tuneParameters();
    const QStringList values = {
        status.connected ? "24" : "0",
        QString::number(parameters.filamentCurrent, 'f', 1),
        QString::number(parameters.electronEnergy, 'f', 1),
        QString::number(parameters.filamentCurrent, 'f', 1),
        QString::number(scanSettings().massEnd, 'f', 1),
        QString::number(scanSettings().sampleRateHz / 1000.0, 'f', 1),
        QString::number(parameters.multiplierVoltage, 'f', 1),
        QString::number(parameters.preQuadrupoleFrontVoltage, 'f', 1),
        QString::number(parameters.preQuadrupoleRearVoltage, 'f', 1),
        QString::number(parameters.repellerVoltage, 'f', 1),
        QString::number(parameters.lens1Voltage, 'f', 1),
        QString::number(parameters.lens2Voltage, 'f', 1)
    };
    statusPanel_->setInstrumentStatus(status, values);
}

void TunePage::setConnectionConfig(const DeviceConnectionConfig &config) {
    connectionIpValueLabel_->setText(config.host);
    connectionPortValueLabel_->setText(QString::number(config.port));
    if (auto *ipEdit = rightTabWidget_->findChild<QLineEdit *>("tuneConnectionIpEdit")) {
        ipEdit->setText(config.host);
    }
    if (auto *portEdit = rightTabWidget_->findChild<QLineEdit *>("tuneConnectionPortEdit")) {
        portEdit->setText(QString::number(config.port));
    }
}

void TunePage::setFrame(const SpectrumFrame &frame) {
    dualSpectrumPanel_->setSpectrumFrame(frame);
    const QList<double> refMasses = frame.scanMode == ScanMode::SelectedIon
        ? QList<double>{69.0, 131.0, 219.0}
        : QList<double>{18.0, 28.0, 44.0};
    summaryTable_->setHorizontalHeaderLabels({"m/z",
                                              QString::number(refMasses[0], 'f', 0),
                                              QString::number(refMasses[1], 'f', 0),
                                              QString::number(refMasses[2], 'f', 0)});
    for (int i = 0; i < refMasses.size(); ++i) {
        double peakHeight = 0.0;
        for (const auto &peak : frame.peaks) {
            if (qAbs(peak.mass - refMasses[i]) < 1.5) {
                peakHeight = peak.intensity;
                break;
            }
        }
        summaryTable_->setItem(0, i + 1, new QTableWidgetItem(QString::number(peakHeight, 'f', 1)));
        summaryTable_->setItem(1, i + 1, new QTableWidgetItem(QString::number(peakHeight / 20.0, 'f', 1)));
        summaryTable_->setItem(2, i + 1, new QTableWidgetItem(QString::number(1.2 + i * 0.2, 'f', 2)));
    }
}

void TunePage::applyChartSettings(const ChartSettings &settings) {
    dualSpectrumPanel_->applyChartSettings(settings);
}

void TunePage::setScanState(bool connected, bool scanning) {
    const bool editable = connected && !scanning;
    scanModeBox_->setEnabled(editable);
    detectorBox_->setEnabled(editable);
    massStartSpin_->setEnabled(editable);
    massEndSpin_->setEnabled(editable);
    resolutionSpin_->setEnabled(editable);
    scanTimeSpin_->setEnabled(editable);
    sampleRateSpin_->setEnabled(editable);
    targetIonsEdit_->setEnabled(editable);
    fullScanCheck_->setEnabled(editable);
    simCheck_->setEnabled(editable);
    detectorMultiplierCheck_->setEnabled(editable);
    detectorFaradayCheck_->setEnabled(editable);
    stableZoneOneCheck_->setEnabled(editable);
    stableZoneTwoCheck_->setEnabled(editable);
    scanSettingsButton_->setEnabled(editable);
    for (auto *check : targetIonChecks_) {
        check->setEnabled(editable && scanModeBox_->currentIndex() == 1);
    }
    targetIonBox_->setVisible(scanModeBox_->currentIndex() == 1);
    repellerVoltageRow_->setEnabled(editable);
    lens1VoltageRow_->setEnabled(editable);
    lens2VoltageRow_->setEnabled(editable);
    highMassCompensationRow_->setEnabled(editable);
    lowMassCompensationRow_->setEnabled(editable);
    multiplierVoltageRow_->setEnabled(editable);
    rodVoltageRow_->setEnabled(editable);
    eVoltageRow_->setEnabled(editable);
    electronEnergyRow_->setEnabled(editable);
    filamentCurrentRow_->setEnabled(editable);
    outerDeflectionLensVoltageRow_->setEnabled(editable);
    innerDeflectionLensVoltageRow_->setEnabled(editable);
    preQuadrupoleFrontVoltageRow_->setEnabled(editable);
    preQuadrupoleRearVoltageRow_->setEnabled(editable);
    showAdvancedParametersCheck_->setEnabled(editable);
    applyParametersButton_->setEnabled(editable);
    if (connectDeviceButton_) {
        connectDeviceButton_->setEnabled(!scanning);
    }
    if (disconnectDeviceButton_) {
        disconnectDeviceButton_->setEnabled(connected && !scanning);
    }

    actionHeader_->setActionEnabled(HeaderAction::Start, editable);
    actionHeader_->setActionEnabled(HeaderAction::Stop, connected && scanning);
    actionHeader_->setActionEnabled(HeaderAction::Calibrate, editable);
    actionHeader_->setActionEnabled(HeaderAction::SaveData, true);
    actionHeader_->setActionEnabled(HeaderAction::RefreshChart, true);
    actionHeader_->setActionEnabled(HeaderAction::FilamentSwitch, connected && !scanning);
    actionHeader_->setSwitchEnabled(InstrumentSwitch::ForePump, connected);
    actionHeader_->setSwitchEnabled(InstrumentSwitch::ForeValve, connected);
    actionHeader_->setSwitchEnabled(InstrumentSwitch::MolecularPump, connected);
    actionHeader_->setSwitchEnabled(InstrumentSwitch::InletValve, connected);
    actionHeader_->setSwitchEnabled(InstrumentSwitch::Filament, connected);
    actionHeader_->setSwitchEnabled(InstrumentSwitch::Multiplier, connected);

    if (!connected) {
        modeHintLabel_->setText("设备未连接，请先建立连接。");
    } else if (scanning) {
        modeHintLabel_->setText("扫描进行中，参数已锁定。停止扫描后才能修改。");
    } else {
        modeHintLabel_->setText("当前未扫描，可编辑扫描设置和调谐参数。");
    }
}

void TunePage::toggleDetectorMode() {
    detectorBox_->setCurrentIndex(detectorBox_->currentIndex() == 0 ? 1 : 0);
}

void TunePage::syncTargetIonSelection() {
    QStringList selected;
    const bool simMode = scanModeBox_->currentIndex() == 1;
    targetIonBox_->setVisible(simMode);
    for (auto *check : targetIonChecks_) {
        check->setEnabled(simMode);
        if (check->isChecked()) {
            selected << check->text();
        }
    }
    if (!simMode) {
        targetIonsEdit_->setText("18,28,44");
        return;
    }
    if (selected.isEmpty()) {
        targetIonChecks_.first()->blockSignals(true);
        targetIonChecks_.first()->setChecked(true);
        targetIonChecks_.first()->blockSignals(false);
        selected << targetIonChecks_.first()->text();
    }
    targetIonsEdit_->setText(selected.join(","));
}

}  // namespace deviceapp
