#include "ui/pages/monitor_page.h"

#include <QButtonGroup>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace deviceapp {

namespace {

QString ionColor(int index) {
    static const QStringList colors = {"#ff4d4f", "#3b82f6", "#8b5cf6", "#f59e0b", "#10b981"};
    return colors[index % colors.size()];
}

const QString kMethodBarStyle =
    "QFrame { background: #d7d2ef; border: 1px solid #b9b2dc; }"
    "QPushButton { min-height: 28px; min-width: 96px; padding: 0 14px; "
    "border: 1px solid #9d97bc; border-radius: 2px; background: #ebe8f8; color: #373552; font-size: 13px; }"
    "QPushButton:hover { background: #f3f1fb; }"
    "QPushButton:pressed { background: #ddd8f1; }"
    "QPushButton:checked { background: #cfc8ea; }"
    "QPushButton:disabled { background: #eceaf5; color: #9793aa; border-color: #c6c2d7; }"
    "QLabel { color: #f8f7ff; font-size: 14px; font-weight: 600; padding-right: 4px; }";

const QString kRightPanelStyle = "QFrame { background: #f6f6f7; border: 1px solid #c9ccd2; }";

const QString kRightTabsStyle =
    "QTabWidget::pane { border: none; top: -1px; background: #ffffff; }"
    "QTabBar::tab { min-width: 88px; min-height: 28px; padding: 4px 10px; "
    "background: #ececec; border: 1px solid #c9ccd2; border-bottom: 1px solid #c9ccd2; color: #4b5563; }"
    "QTabBar::tab:selected { background: #ffffff; color: #222222; border-bottom-color: #ffffff; font-weight: 600; }"
    "QTabBar::tab:!selected { margin-top: 1px; }";

const QString kGroupStyle =
    "QGroupBox { border: 1px solid #d8dbe2; margin-top: 8px; font-weight: 600; background: #ffffff; color: #384250; }"
    "QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; color: #4b5563; }"
    "QRadioButton, QCheckBox { spacing: 6px; color: #344054; }";

const QString kSimFrameStyle = "QFrame { border: 1px solid #d8dbe2; background: #ffffff; }";
const QString kSecondaryButtonStyle =
    "QPushButton { min-height: 28px; padding: 0 14px; background: #f5f6f8; border: 1px solid #c5c9d0; border-radius: 6px; color: #384250; }"
    "QPushButton:hover { background: #eef1f5; }"
    "QPushButton:pressed { background: #e4e8ee; }"
    "QPushButton:disabled { color: #98a2b3; background: #f6f7f9; border-color: #d8dbe2; }";

}

MonitorPage::MonitorPage(QWidget *parent) : QWidget(parent) {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    actionHeader_ = new PageActionHeader(this);
    actionHeader_->hide();
    mainLayout->addWidget(actionHeader_);

    alarmModeButton_ = nullptr;
    analysisModeButton_ = nullptr;

    auto *methodBar = new QFrame(this);
    methodBar->setStyleSheet(kMethodBarStyle);
    auto *methodBarLayout = new QHBoxLayout(methodBar);
    methodBarLayout->setContentsMargins(12, 7, 12, 7);
    methodBarLayout->setSpacing(10);
    openMethodButton_ = new QPushButton("打开方法", this);
    runMethodButton_ = new QPushButton("运行方法", this);
    pauseRefreshButton_ = new QPushButton("暂停刷新", this);
    pauseRefreshButton_->setCheckable(true);
    saveDataButton_ = new QPushButton("数据另存", this);
    toolbarStatusLabel_ = new QLabel("分析模式运行", this);
    methodBarLayout->addWidget(openMethodButton_);
    methodBarLayout->addWidget(runMethodButton_);
    methodBarLayout->addWidget(pauseRefreshButton_);
    methodBarLayout->addWidget(saveDataButton_);
    methodBarLayout->addStretch();
    methodBarLayout->addWidget(toolbarStatusLabel_);
    mainLayout->addWidget(methodBar);

    auto *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(0);
    mainLayout->addLayout(contentLayout, 1);

    auto *chartPanel = new QFrame(this);
    chartPanel->setStyleSheet("QFrame { background: white; border: 1px solid #a9adb7; }");
    auto *chartLayout = new QVBoxLayout(chartPanel);
    chartLayout->setContentsMargins(0, 0, 0, 0);
    chartLayout->setSpacing(2);

    ricChart_ = new QChart();
    ricChart_->setTitle("RIC");
    ricChart_->legend()->setVisible(true);
    ricChart_->legend()->setAlignment(Qt::AlignRight);
    ricAxisX_ = new QValueAxis(this);
    ricAxisY_ = new QValueAxis(this);
    ricAxisX_->setRange(0.0, 1000.0);
    ricAxisY_->setRange(0.0, 1000.0);
    ricAxisX_->setTitleText("s");
    ricAxisY_->setTitleText("总压");
    ricChart_->addAxis(ricAxisX_, Qt::AlignBottom);
    ricChart_->addAxis(ricAxisY_, Qt::AlignLeft);
    ricChartView_ = new QChartView(ricChart_, this);
    ricChartView_->setRenderHint(QPainter::Antialiasing);

    ticChart_ = new QChart();
    ticChart_->setTitle("TIC");
    ticChart_->legend()->hide();
    ticAxisX_ = new QValueAxis(this);
    ticAxisY_ = new QValueAxis(this);
    ticAxisX_->setRange(0.0, 1000.0);
    ticAxisY_->setRange(0.0, 1000.0);
    ticAxisX_->setTitleText("s");
    ticAxisY_->setTitleText("总压");
    ticChart_->addAxis(ticAxisX_, Qt::AlignBottom);
    ticChart_->addAxis(ticAxisY_, Qt::AlignLeft);
    ticChartView_ = new QChartView(ticChart_, this);
    ticChartView_->setRenderHint(QPainter::Antialiasing);

    chartLayout->addWidget(ricChartView_, 5);
    chartLayout->addWidget(ticChartView_, 2);

    auto *rightPanel = new QFrame(this);
    rightPanel->setMinimumWidth(300);
    rightPanel->setMaximumWidth(320);
    rightPanel->setStyleSheet(kRightPanelStyle);
    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);
    rightTabs_ = new QTabWidget(this);
    rightTabs_->setStyleSheet(kRightTabsStyle);

    parameterTab_ = new QWidget(this);
    auto *parameterLayout = new QVBoxLayout(parameterTab_);
    parameterLayout->setContentsMargins(8, 6, 8, 8);
    parameterLayout->setSpacing(6);

    auto *detectorBox = new QGroupBox("检测器", this);
    detectorBox->setStyleSheet(kGroupStyle);
    auto *detectorLayout = new QHBoxLayout(detectorBox);
    detectorLayout->setContentsMargins(10, 8, 10, 8);
    detectorLayout->setSpacing(12);
    detectorMultiplierRadio_ = new QRadioButton("倍增器", this);
    detectorFaradayRadio_ = new QRadioButton("法拉第盘", this);
    detectorMultiplierRadio_->setChecked(true);
    auto *detectorGroup = new QButtonGroup(this);
    detectorGroup->addButton(detectorMultiplierRadio_);
    detectorGroup->addButton(detectorFaradayRadio_);
    detectorLayout->addWidget(detectorMultiplierRadio_);
    detectorLayout->addWidget(detectorFaradayRadio_);
    detectorLayout->addStretch();
    parameterLayout->addWidget(detectorBox);

    auto *stabilityBox = new QGroupBox("稳定区", this);
    stabilityBox->setStyleSheet(kGroupStyle);
    auto *stabilityLayout = new QHBoxLayout(stabilityBox);
    stabilityLayout->setContentsMargins(10, 8, 10, 8);
    stabilityLayout->setSpacing(12);
    stabilityZoneOneRadio_ = new QRadioButton("稳定区一", this);
    stabilityZoneTwoRadio_ = new QRadioButton("稳定区二", this);
    stabilityZoneOneRadio_->setChecked(true);
    auto *stabilityGroup = new QButtonGroup(this);
    stabilityGroup->addButton(stabilityZoneOneRadio_);
    stabilityGroup->addButton(stabilityZoneTwoRadio_);
    stabilityLayout->addWidget(stabilityZoneOneRadio_);
    stabilityLayout->addWidget(stabilityZoneTwoRadio_);
    stabilityLayout->addStretch();
    parameterLayout->addWidget(stabilityBox);

    auto *scanModeBox = new QGroupBox("扫描方式", this);
    scanModeBox->setStyleSheet(kGroupStyle);
    auto *scanModeLayout = new QHBoxLayout(scanModeBox);
    scanModeLayout->setContentsMargins(10, 8, 10, 8);
    scanModeLayout->setSpacing(12);
    fullScanRadio_ = new QRadioButton("Full", this);
    simRadio_ = new QRadioButton("SIM", this);
    simRadio_->setChecked(true);
    auto *scanModeGroup = new QButtonGroup(this);
    scanModeGroup->addButton(fullScanRadio_);
    scanModeGroup->addButton(simRadio_);
    scanModeLayout->addWidget(fullScanRadio_);
    scanModeLayout->addWidget(simRadio_);
    scanModeLayout->addStretch();
    parameterLayout->addWidget(scanModeBox);

    simSettingsFrame_ = new QFrame(this);
    simSettingsFrame_->setStyleSheet(kSimFrameStyle);
    auto *simLayout = new QVBoxLayout(simSettingsFrame_);
    simLayout->setContentsMargins(8, 8, 8, 8);
    simLayout->setSpacing(8);
    auto *simTitle = new QLabel("SIM扫描设置", this);
    simTitle->setStyleSheet("QLabel { font-weight: 600; color: #4b5563; padding: 0 0 2px 0; }");
    simLayout->addWidget(simTitle);

    dwellTimeEdit_ = new QLineEdit("5", this);
    flybackTimeEdit_ = new QLineEdit("100", this);
    peakWidthEdit_ = new QLineEdit("1", this);
    rampVoltageEdit_ = new QLineEdit("5", this);
    dwellTimeEdit_->hide();
    flybackTimeEdit_->hide();
    peakWidthEdit_->hide();
    rampVoltageEdit_->hide();

    simTable_ = new QTableWidget(5, 4, this);
    simTable_->setHorizontalHeaderLabels({"选择", "质量数", "质程宽度", "时间s"});
    simTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    simTable_->horizontalHeader()->setMinimumHeight(28);
    simTable_->verticalHeader()->setDefaultSectionSize(28);
    simTable_->verticalHeader()->setVisible(false);
    simTable_->setSelectionMode(QAbstractItemView::NoSelection);
    simTable_->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed);
    simTable_->setMinimumHeight(228);
    simTable_->setStyleSheet(
        "QTableWidget { background: white; gridline-color: #d9dbe1; border: 1px solid #cfd4dc; color: #344054; }"
        "QTableWidget::item { padding: 2px 4px; }"
        "QHeaderView::section { background: #f6f7f9; border: 1px solid #d9dbe1; padding: 4px 3px; color: #475467; }");
    const QList<int> defaultIons = {79, 94, 109, 124};
    for (int row = 0; row < simTable_->rowCount(); ++row) {
        auto *check = new QCheckBox(this);
        check->setChecked(row < defaultIons.size());
        ionEnabledChecks_.append(check);
        auto *checkCell = new QWidget(this);
        auto *checkLayout = new QHBoxLayout(checkCell);
        checkLayout->setContentsMargins(0, 0, 0, 0);
        checkLayout->addWidget(check, 0, Qt::AlignCenter);
        simTable_->setCellWidget(row, 0, checkCell);
        simTable_->setItem(row, 1, new QTableWidgetItem(row < defaultIons.size() ? QString::number(defaultIons[row]) : ""));
        simTable_->setItem(row, 2, new QTableWidgetItem(row < defaultIons.size() ? "1" : ""));
        simTable_->setItem(row, 3, new QTableWidgetItem(row < defaultIons.size() ? "5" : ""));
    }
    simLayout->addWidget(simTable_);

    deleteSelectedButton_ = new QPushButton("删除选中", this);
    deleteSelectedButton_->setFixedSize(112, 28);
    deleteSelectedButton_->setStyleSheet(kSecondaryButtonStyle);

    auto *settingRow = new QHBoxLayout();
    settingRow->setContentsMargins(0, 0, 0, 0);
    settingRow->setSpacing(10);
    parameterSettingsButton_ = new QPushButton("设置", this);
    parameterSettingsButton_->setFixedSize(112, 28);
    parameterSettingsButton_->setStyleSheet(kSecondaryButtonStyle);
    settingRow->addWidget(deleteSelectedButton_);
    settingRow->addWidget(parameterSettingsButton_);
    settingRow->addStretch();
    simLayout->addLayout(settingRow);
    parameterLayout->addWidget(simSettingsFrame_);
    parameterLayout->addStretch();

    statusPanel_ = new InstrumentStatusPanel(this);

    rightTabs_->addTab(parameterTab_, "参数配置");
    rightTabs_->addTab(statusPanel_, "仪器状态");
    rightLayout->addWidget(rightTabs_);

    contentLayout->addWidget(chartPanel, 1);
    contentLayout->addWidget(rightPanel);

    auto *bottomBar = new QFrame(this);
    bottomBar->setStyleSheet("QFrame { background: #f6f3d7; border: 1px solid #c7c19a; }");
    auto *bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(8, 4, 8, 4);
    modeHintLabel_ = new QLabel("运行状态", this);
    runStatusLabel_ = new QLabel("设备未连接", this);
    bottomLayout->addWidget(modeHintLabel_);
    bottomLayout->addStretch();
    bottomLayout->addWidget(runStatusLabel_);
    mainLayout->addWidget(bottomBar);

    connect(fullScanRadio_, &QRadioButton::toggled, this, [this](bool) { syncModeSections(); });
    connect(simRadio_, &QRadioButton::toggled, this, [this](bool) { syncModeSections(); });
    connect(openMethodButton_, &QPushButton::clicked, this, [this]() { emit methodLoadRequested(QString()); });
    connect(runMethodButton_, &QPushButton::clicked, this, [this]() {
        if (runMethodButton_->text() == "停止方法") {
            emit stopRequested();
            return;
        }
        emit startRequested(currentMethod());
    });
    connect(actionHeader_, &PageActionHeader::saveRequested, this, [this]() { emit methodSaveRequested(currentMethod()); });
    connect(saveDataButton_, &QPushButton::clicked, this, [this]() { emit methodSaveRequested(currentMethod()); });
    connect(pauseRefreshButton_, &QPushButton::toggled, this, [this](bool checked) {
        displayPaused_ = checked;
        pauseRefreshButton_->setText(checked ? "恢复刷新" : "暂停刷新");
    });
    connect(parameterSettingsButton_, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "提示", "请通过菜单【设置】-【扫描设置】调整详细参数。");
    });
    connect(deleteSelectedButton_, &QPushButton::clicked, this, [this]() {
        for (int row = simTable_->rowCount() - 1; row >= 0; --row) {
            if (row < ionEnabledChecks_.size() && ionEnabledChecks_[row]->isChecked()) {
                simTable_->removeRow(row);
                ionEnabledChecks_.removeAt(row);
            }
        }
        while (simTable_->rowCount() < 5) {
            const int row = simTable_->rowCount();
            simTable_->insertRow(row);
            auto *check = new QCheckBox(this);
            ionEnabledChecks_.append(check);
            auto *checkCell = new QWidget(this);
            auto *checkLayout = new QHBoxLayout(checkCell);
            checkLayout->setContentsMargins(0, 0, 0, 0);
            checkLayout->addWidget(check, 0, Qt::AlignCenter);
            simTable_->setCellWidget(row, 0, checkCell);
            simTable_->setItem(row, 1, new QTableWidgetItem());
            simTable_->setItem(row, 2, new QTableWidgetItem());
            simTable_->setItem(row, 3, new QTableWidgetItem());
        }
    });

    actionHeader_->setActionEnabled(HeaderAction::Calibrate, false);
    actionHeader_->setActionEnabled(HeaderAction::FilamentSwitch, false);
    actionHeader_->setActionVisible(HeaderAction::RefreshChart, false);
    actionHeader_->setSwitchEnabled(InstrumentSwitch::ForePump, false);
    actionHeader_->setSwitchEnabled(InstrumentSwitch::ForeValve, false);
    actionHeader_->setSwitchEnabled(InstrumentSwitch::MolecularPump, false);
    actionHeader_->setSwitchEnabled(InstrumentSwitch::InletValve, false);
    actionHeader_->setSwitchEnabled(InstrumentSwitch::Filament, false);
    actionHeader_->setSwitchEnabled(InstrumentSwitch::Multiplier, false);
    syncModeSections();
    setScanState(false, false);
}

QVector<double> MonitorPage::selectedIons() const {
    QVector<double> ions;
    for (int row = 0; row < simTable_->rowCount(); ++row) {
        if (row >= ionEnabledChecks_.size() || !ionEnabledChecks_[row]->isChecked()) {
            continue;
        }
        if (auto *item = simTable_->item(row, 1)) {
            const QString text = item->text().trimmed();
            if (!text.isEmpty()) {
                ions.append(text.toDouble());
            }
        }
    }
    return ions;
}

MonitorMethod MonitorPage::currentMethod() const {
    MonitorMethod method;
    method.name = "Default";
    method.scanSettings.mode = fullScanRadio_->isChecked() ? ScanMode::FullScan : ScanMode::SelectedIon;
    method.scanSettings.massStart = 1.0;
    method.scanSettings.massEnd = 100.0;
    method.scanSettings.targetDwellTimeMs = dwellTimeEdit_->text().toDouble();
    method.scanSettings.flybackTimeMs = flybackTimeEdit_->text().toDouble();
    method.scanSettings.targetPeakWidth = peakWidthEdit_->text().toDouble();
    method.scanSettings.rampVoltage = rampVoltageEdit_->text().toDouble();
    method.scanSettings.targetIons = selectedIons();
    method.detector = detectorMultiplierRadio_->isChecked() ? DetectorType::ElectronMultiplier : DetectorType::FaradayCup;
    method.dwellTimeMs = method.scanSettings.targetDwellTimeMs;
    return method;
}

void MonitorPage::applyMethod(const MonitorMethod &method) {
    fullScanRadio_->setChecked(method.scanSettings.mode == ScanMode::FullScan);
    simRadio_->setChecked(method.scanSettings.mode == ScanMode::SelectedIon);
    detectorMultiplierRadio_->setChecked(method.detector == DetectorType::ElectronMultiplier);
    detectorFaradayRadio_->setChecked(method.detector == DetectorType::FaradayCup);
    const QVector<double> ions = method.scanSettings.targetIons.isEmpty() ? QVector<double>{79.0, 94.0, 109.0, 124.0} : method.scanSettings.targetIons;
    simTable_->setRowCount(qMax(5, ions.size() + 1));
    ionEnabledChecks_.clear();
    for (int row = 0; row < simTable_->rowCount(); ++row) {
        auto *check = new QCheckBox(this);
        check->setChecked(row < ions.size());
        ionEnabledChecks_.append(check);
        auto *checkCell = new QWidget(this);
        auto *checkLayout = new QHBoxLayout(checkCell);
        checkLayout->setContentsMargins(0, 0, 0, 0);
        checkLayout->addWidget(check, 0, Qt::AlignCenter);
        simTable_->setCellWidget(row, 0, checkCell);
        simTable_->setItem(row, 1, new QTableWidgetItem(row < ions.size() ? QString::number(ions[row], 'f', ions[row] == qRound64(ions[row]) ? 0 : 2) : ""));
        simTable_->setItem(row, 2, new QTableWidgetItem(row < ions.size() ? "1" : ""));
        simTable_->setItem(row, 3, new QTableWidgetItem(row < ions.size() ? "5" : ""));
    }
    syncModeSections();
}

void MonitorPage::rebuildRicChart(const SpectrumFrame &frame) {
    ricChart_->removeAllSeries();
    const QVector<double> ions = selectedIons().isEmpty() ? QVector<double>{79.0, 94.0, 109.0, 124.0} : selectedIons();
    for (int ionIndex = 0; ionIndex < ions.size(); ++ionIndex) {
        auto *series = new QLineSeries(this);
        series->setName(QString::number(ions[ionIndex], 'f', 0));
        series->setColor(QColor(ionColor(ionIndex)));
        for (int i = 0; i < 60; ++i) {
            const double x = i * (1000.0 / 59.0);
            const double base = frame.intensities.isEmpty() ? 0.0 : frame.intensities[i % frame.intensities.size()];
            const double intensity = qBound(0.0, base * (0.4 + ionIndex * 0.15) + (ionIndex + 1) * 8.0, 1000.0);
            series->append(x, intensity);
        }
        ricChart_->addSeries(series);
        series->attachAxis(ricAxisX_);
        series->attachAxis(ricAxisY_);
    }
    ricAxisX_->setRange(0.0, 1000.0);
    ricAxisY_->setRange(0.0, 1000.0);
}

void MonitorPage::rebuildTicChart(const SpectrumFrame &frame) {
    ticChart_->removeAllSeries();
    auto *series = new QLineSeries(this);
    series->setColor(QColor("#3b3b3b"));
    for (int i = 0; i < 60; ++i) {
        const double x = i * (1000.0 / 59.0);
        const double base = frame.intensities.isEmpty() ? 0.0 : frame.intensities[i % frame.intensities.size()];
        series->append(x, qBound(0.0, base * 0.3 + (i % 7) * 4.0, 1000.0));
    }
    ticChart_->addSeries(series);
    series->attachAxis(ticAxisX_);
    series->attachAxis(ticAxisY_);
    ticAxisX_->setRange(0.0, 1000.0);
    ticAxisY_->setRange(0.0, 1000.0);
}

void MonitorPage::setFrame(const SpectrumFrame &frame) {
    lastFrame_ = frame;
    if (!displayPaused_) {
        rebuildRicChart(frame);
        rebuildTicChart(frame);
    }
    runStatusLabel_->setText(QString("真空度：进样 %1Pa").arg(QString::number(frame.parameterSnapshot.value("vacuum").toDouble(2.7e-3), 'e', 1)));
}

void MonitorPage::setAvailableMethods(const QStringList &files) {
    Q_UNUSED(files);
}

void MonitorPage::applyChartSettings(const ChartSettings &settings) {
    Q_UNUSED(settings);
}

void MonitorPage::setInstrumentStatus(const InstrumentStatus &status) {
    const QVector<double> ions = selectedIons();
    const QStringList values = {
        status.connected ? "24" : "0",
        QString::number(0.0, 'f', 1),
        QString::number(0.0, 'f', 1),
        QString::number(0.0, 'f', 1),
        fullScanRadio_->isChecked() ? QString::number(100.0, 'f', 1) : QString::number(ions.isEmpty() ? 0.0 : ions.first(), 'f', 1),
        QString::number(20.0, 'f', 1),
        detectorMultiplierRadio_->isChecked() ? QString::number(1200.0, 'f', 1) : QString::number(0.0, 'f', 1),
        QString::number(0.0, 'f', 1),
        QString::number(0.0, 'f', 1),
        QString::number(0.0, 'f', 1),
        QString::number(0.0, 'f', 1),
        QString::number(0.0, 'f', 1)
    };
    statusPanel_->setInstrumentStatus(status, values);
}

void MonitorPage::setScanState(bool connected, bool scanning) {
    const bool editable = connected && !scanning;
    fullScanRadio_->setEnabled(editable);
    simRadio_->setEnabled(editable);
    detectorMultiplierRadio_->setEnabled(editable);
    detectorFaradayRadio_->setEnabled(editable);
    stabilityZoneOneRadio_->setEnabled(editable);
    stabilityZoneTwoRadio_->setEnabled(editable);
    openMethodButton_->setEnabled(editable);
    parameterSettingsButton_->setEnabled(editable);
    deleteSelectedButton_->setEnabled(editable && simRadio_->isChecked());
    simTable_->setEnabled(editable && simRadio_->isChecked());
    pauseRefreshButton_->setEnabled(connected);
    saveDataButton_->setEnabled(connected);
    runMethodButton_->setEnabled(connected);
    runMethodButton_->setText(scanning ? "停止方法" : "运行方法");
    toolbarStatusLabel_->setText("分析模式运行");

    if (!connected) {
        runStatusLabel_->setText("设备未连接");
    } else if (scanning) {
        runStatusLabel_->setText("监测运行中");
    } else {
        runStatusLabel_->setText("开始扫描");
    }
}

void MonitorPage::syncModeSections() {
    simSettingsFrame_->setVisible(simRadio_->isChecked());
    deleteSelectedButton_->setEnabled(simRadio_->isChecked());
}

}  // namespace deviceapp
