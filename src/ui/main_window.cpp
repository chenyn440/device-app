#include "ui/main_window.h"

#include <QAction>
#include <QApplication>
#include <QSettings>
#include <QDockWidget>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QCloseEvent>
#include <QPushButton>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTimer>
#include <QToolButton>
#include <QToolBar>
#include <QVBoxLayout>

#include "core/app_settings.h"
#include "ui/dialogs/chart_settings_dialog.h"
#include "ui/dialogs/connection_dialog.h"
#include "ui/dialogs/data_processing_dialog.h"
#include "ui/dialogs/frame_viewer_dialog.h"
#include "ui/dialogs/instrument_control_dialog.h"
#include "ui/dialogs/scan_settings_dialog.h"
#include "ui/dialogs/system_settings_dialog.h"
#include "ui/pages/data_page.h"
#include "ui/pages/monitor_page.h"
#include "ui/pages/settings_page.h"
#include "ui/pages/tune_page.h"
#include "ui/pages/workbench_placeholder_page.h"

namespace deviceapp {

namespace {

constexpr int kStatusRefreshIntervalMs = 1000;
constexpr bool kAutoReconnectEnabled = true;
constexpr bool kRememberWindowLayout = true;

}  // namespace

MainWindow::MainWindow(ApplicationContext *context, QWidget *parent)
    : QMainWindow(parent), context_(context) {
    setupUi();
    setupMenus();
    setupToolbar();
    wireSignals();
    applySystemSettings(context_->settingsService->systemSettings());
    restoreWindowStateFromSettings();

    statusBar()->showMessage("就绪");
    monitorPage_->setAvailableMethods(context_->monitorService->listMethods());
    tunePage_->setScanState(false, false);
    monitorPage_->setScanState(false, false);
    updateActionStates(currentStatus_);
}

void MainWindow::setupUi() {
    setWindowTitle("四极质谱上位机");
    menuBar()->hide();

    auto *central = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    menuBarContainer_ = new QWidget(this);
    menuBarContainer_->setStyleSheet("QWidget { background: #f5f7fa; border-bottom: 1px solid #d0d5dd; }");
    auto *menuBarLayout = new QHBoxLayout(menuBarContainer_);
    menuBarLayout->setContentsMargins(16, 6, 16, 6);
    menuBarLayout->setSpacing(2);
    const QString topButtonStyle =
        "QPushButton { background: transparent; border: none; padding: 6px 12px; color: #101828; font-size: 13px; }"
        "QPushButton:hover { background: #e7edf5; }"
        "QPushButton:pressed { background: #dbe7f3; }";
    for (const QString &label : {"文件", "运行", "设置", "视图"}) {
        auto *button = new QPushButton(label, this);
        button->setStyleSheet(topButtonStyle);
        button->setFlat(true);
        menuBarLayout->addWidget(button, 0, Qt::AlignLeft);
        if (label == "文件") {
            connect(button, &QPushButton::clicked, this, [this, button]() { showMenuBelowButton(fileMenu_, button); });
        } else if (label == "运行") {
            connect(button, &QPushButton::clicked, this, [this, button]() { showMenuBelowButton(runMenu_, button); });
        } else if (label == "设置") {
            connect(button, &QPushButton::clicked, this, [this, button]() { showMenuBelowButton(settingsMenu_, button); });
        } else if (label == "视图") {
            connect(button, &QPushButton::clicked, this, [this, button]() { showMenuBelowButton(viewMenu_, button); });
        }
    }
    menuBarLayout->addStretch();
    rootLayout->addWidget(menuBarContainer_);

    auto *contentRow = new QHBoxLayout();
    contentRow->setContentsMargins(0, 0, 0, 0);
    contentRow->setSpacing(0);
    rootLayout->addLayout(contentRow, 1);

    navigation_ = new QListWidget(this);
    navigation_->addItems({"调谐", "监测", "方法", "数据", "设置"});
    for (int i = 0; i < navigation_->count(); ++i) {
        if (auto *item = navigation_->item(i)) {
            item->setTextAlignment(Qt::AlignCenter);
        }
    }
    navigation_->setFixedWidth(66);
    navigation_->setSpacing(4);
    navigation_->setStyleSheet(
        "QListWidget { background: #f2f4f7; border: 1px solid #d0d5dd; border-top: none; padding: 0; }"
        "QListWidget::item { min-height: 48px; border: none; text-align: center; padding: 4px 0; color: #344054; background: transparent; }"
        "QListWidget::item:hover { background: #eef2f6; border: none; }"
        "QListWidget::item:selected { background: #eaf3ff; border: none; color: #175cd3; }");

    rightPane_ = new QWidget(this);
    auto *rightPaneLayout = new QVBoxLayout(rightPane_);
    rightPaneLayout->setContentsMargins(0, 0, 0, 0);
    rightPaneLayout->setSpacing(0);

    stack_ = new QStackedWidget(this);
    tunePage_ = new TunePage(context_->scanControlService.get(), context_->tuneService.get(), this);
    monitorPage_ = new MonitorPage(this);
    methodPage_ = new WorkbenchPlaceholderPage("方法管理", "这里将承载调谐参数模板、监测方法列表、导入导出与版本管理。", this);
    dataPage_ = new DataPage(this);
    settingsPage_ = new SettingsPage(this);
    stack_->addWidget(tunePage_);
    stack_->addWidget(monitorPage_);
    stack_->addWidget(methodPage_);
    stack_->addWidget(dataPage_);
    stack_->addWidget(settingsPage_);

    rightPaneLayout->addWidget(stack_, 1);
    contentRow->addWidget(navigation_);
    contentRow->addWidget(rightPane_, 1);
    setCentralWidget(central);

    connectionDialog_ = new ConnectionDialog(this);
    connectionDialog_->setConfig(context_->connectionService->recentConnection());
    scanSettingsDialog_ = new ScanSettingsDialog(this);
    instrumentControlDialog_ = new InstrumentControlDialog(this);
    dataProcessingDialog_ = new DataProcessingDialog(this);
    systemSettingsDialog_ = new SystemSettingsDialog(this);
    chartSettingsDialog_ = new ChartSettingsDialog(this);

    instrumentControlDialog_->setSettings(context_->settingsService->instrumentControlSettings());
    dataProcessingDialog_->setSettings(context_->settingsService->dataProcessingSettings());
    systemSettingsDialog_->setSettings(context_->settingsService->systemSettings());
    chartSettingsDialog_->setSettings(context_->settingsService->chartSettings());
    context_->deviceAdapter->applyDataProcessingSettings(context_->settingsService->dataProcessingSettings());
    tunePage_->applyChartSettings(context_->settingsService->chartSettings());
    tunePage_->setConnectionConfig(context_->connectionService->recentConnection());
    monitorPage_->applyChartSettings(context_->settingsService->chartSettings());
    dataPage_->applyChartSettings(context_->settingsService->chartSettings());
    settingsPage_->setConnectionConfig(context_->connectionService->recentConnection());
    lastConnectionConfig_ = context_->connectionService->recentConnection();
    statusRefreshTimer_ = new QTimer(this);

    connect(navigation_, &QListWidget::currentRowChanged, stack_, &QStackedWidget::setCurrentIndex);
    navigation_->setCurrentRow(0);
}

void MainWindow::setupMenus() {
    fileMenu_ = new QMenu("文件", this);
    fileMenu_->addAction("打开单帧数据", this, &MainWindow::openFrameViewer);
    fileMenu_->addAction("退出", this, &QWidget::close);

    runMenu_ = new QMenu("运行", this);
    openConnectionAction_ = runMenu_->addAction("仪器连接", this, &MainWindow::openConnectionDialog);
    disconnectAction_ = runMenu_->addAction("断开连接", [this]() { context_->connectionService->disconnectFromDevice(); });
    openInstrumentControlAction_ = runMenu_->addAction("仪器控制", this, &MainWindow::openInstrumentControlDialog);

    settingsMenu_ = new QMenu("设置", this);
    openScanSettingsAction_ = settingsMenu_->addAction("扫描设置", this, &MainWindow::openScanSettingsDialog);
    openDataProcessingAction_ = settingsMenu_->addAction("数据处理", this, &MainWindow::openDataProcessingDialog);
    openSystemSettingsAction_ = settingsMenu_->addAction("系统设置", this, &MainWindow::openSystemSettingsDialog);
    openChartSettingsAction_ = settingsMenu_->addAction("谱图设置", this, &MainWindow::openChartSettingsDialog);

    viewMenu_ = new QMenu("视图", this);
    viewMenu_->addAction("切换到调谐", [this]() { navigation_->setCurrentRow(0); });
    viewMenu_->addAction("切换到监测", [this]() { navigation_->setCurrentRow(1); });
    viewMenu_->addAction("切换到方法", [this]() { navigation_->setCurrentRow(2); });
    viewMenu_->addAction("切换到数据", [this]() { navigation_->setCurrentRow(3); });
    viewMenu_->addAction("切换到设置", [this]() { navigation_->setCurrentRow(4); });
}

void MainWindow::setupToolbar() {
    toolbar_ = new QToolBar("主工具栏", this);
    toolbar_->setToolButtonStyle(Qt::ToolButtonTextOnly);
    toolbar_->setMovable(false);
    toolbar_->setIconSize(QSize(14, 14));
    toolbar_->setStyleSheet(
        "QToolBar { spacing: 4px; background: #eef2f6; border-bottom: 1px solid #d0d5dd; }"
        "QToolButton { padding: 4px 8px; border: 1px solid #b6bcc6; border-radius: 3px; background: #f8fafc; font-size: 12px; }"
        "QToolButton:checked { background: #1f8f4c; color: white; border-color: #1f8f4c; }"
        "QToolButton:disabled { background: #f3f4f6; color: #b6bec8; border-color: #d5dbe3; }");
    rightPane_->layout()->addWidget(toolbar_);
    toolbar_->hide();

    const QList<QPair<QString, InstrumentSwitch>> switches = {
        {"前级泵", InstrumentSwitch::ForePump},
        {"前级阀", InstrumentSwitch::ForeValve},
        {"分子泵", InstrumentSwitch::MolecularPump},
        {"进样阀", InstrumentSwitch::InletValve},
        {"灯丝", InstrumentSwitch::Filament},
        {"倍增器", InstrumentSwitch::Multiplier},
    };

    startScanAction_ = toolbar_->addAction("开始", [this]() {
        const auto settings = tunePage_->scanSettings();
        const auto parameters = tunePage_->tuneParameters();
        context_->tuneService->applyTuneParameters(parameters);
        context_->scanControlService->startScan(settings);
    });
    stopScanAction_ = toolbar_->addAction("停止", context_->scanControlService.get(), &ScanControlService::stopScan);
    calibrateAction_ = toolbar_->addAction("校正当前质量轴", context_->scanControlService.get(), &ScanControlService::calibrateMassAxis);

    for (const auto &item : switches) {
        const QString label = item.first;
        const InstrumentSwitch instrumentSwitch = item.second;
        auto *action = toolbar_->addAction(label);
        action->setCheckable(true);
        switchActions_.insert(instrumentSwitch, action);
        connect(action, &QAction::toggled, this, [this, instrumentSwitch](bool checked) {
            context_->deviceAdapter->setSwitchState(instrumentSwitch, checked);
        });
    }

    filamentSwitchAction_ = toolbar_->addAction("灯丝切换", tunePage_, &TunePage::toggleDetectorMode);
    saveFrameAction_ = toolbar_->addAction("数据保存", [this]() {
        if (!lastFrame_.timestamp.isValid()) {
            showError("当前没有可保存的单帧数据");
            return;
        }
        const QString jsonPath = context_->persistenceService->saveFrame(lastFrame_);
        const QString csvPath = context_->persistenceService->exportFrameCsv(lastFrame_);
        statusBar()->showMessage(QString("已保存 JSON: %1 | CSV: %2").arg(jsonPath, csvPath), 5000);
    });
}

void MainWindow::wireSignals() {
    connect(statusRefreshTimer_, &QTimer::timeout, context_->deviceAdapter.get(), &IDeviceAdapter::readStatusSnapshot);
    connect(context_->connectionService.get(), &ConnectionService::recentConnectionChanged, this, [this](const DeviceConnectionConfig &config) {
        lastConnectionConfig_ = config;
        tunePage_->setConnectionConfig(config);
        settingsPage_->setConnectionConfig(config);
    });
    connect(context_->deviceAdapter.get(), &IDeviceAdapter::connectionChanged, this, [this](bool connected) {
        settingsPage_->setConnectionState(connected);
        if (!connected && kAutoReconnectEnabled && !lastConnectionConfig_.host.isEmpty()) {
            QTimer::singleShot(800, this, [this]() {
                if (!currentStatus_.connected) {
                    context_->connectionService->connectToDevice(lastConnectionConfig_);
                }
            });
        }
    });
    connect(context_->deviceAdapter.get(), &IDeviceAdapter::statusUpdated, this, [this](const InstrumentStatus &status) {
        currentStatus_ = status;
        context_->scanControlService->updateInstrumentStatus(status);
        tunePage_->setInstrumentStatus(status);
        monitorPage_->setInstrumentStatus(status);
        tunePage_->setScanState(status.connected, status.scanning);
        monitorPage_->setScanState(status.connected, status.scanning);
        syncToolbarSwitches(status);
        updateStatusBar(status);
        updateActionStates(status);
    });
    connect(context_->deviceAdapter.get(), &IDeviceAdapter::frameUpdated, this, [this](const SpectrumFrame &frame) {
        lastFrame_ = frame;
        tunePage_->setFrame(frame);
        monitorPage_->setFrame(frame);
        updateActionStates(currentStatus_);
    });
    connect(context_->deviceAdapter.get(), &IDeviceAdapter::frameReadyToPersist, this, [this](const SpectrumFrame &frame) {
        lastFrame_ = frame;
        const QString jsonPath = context_->persistenceService->saveFrame(frame);
        const QString csvPath = context_->persistenceService->exportFrameCsv(frame);
        statusBar()->showMessage(QString("单帧已保存到 %1，CSV 已导出到 %2").arg(jsonPath, csvPath), 4000);
    });
    connect(context_->deviceAdapter.get(), &IDeviceAdapter::calibrationFinished, this, [this](bool success, const QString &message) {
        if (!success) {
            showError(message);
            return;
        }
        statusBar()->showMessage(message, 4000);
    });
    connect(context_->deviceAdapter.get(), &IDeviceAdapter::errorOccurred, this, &MainWindow::showError);
    connect(context_->scanControlService.get(), &ScanControlService::validationFailed, this, &MainWindow::showError);

    connect(tunePage_, &TunePage::startRequested, this, [this](const ScanSettings &settings, const TuneParameters &parameters) {
        context_->tuneService->applyTuneParameters(parameters);
        context_->scanControlService->startScan(settings);
    });
    connect(tunePage_, &TunePage::stopRequested, context_->scanControlService.get(), &ScanControlService::stopScan);
    connect(tunePage_, &TunePage::saveRequested, context_->deviceAdapter.get(), &IDeviceAdapter::saveCurrentFrame);
    connect(tunePage_, &TunePage::refreshRequested, this, [this]() {
        tunePage_->setFrame(lastFrame_);
        statusBar()->showMessage("谱图已刷新", 2000);
    });
    connect(tunePage_, &TunePage::calibrationRequested, context_->scanControlService.get(), &ScanControlService::calibrateMassAxis);
    connect(tunePage_, &TunePage::scanSettingsRequested, this, &MainWindow::openScanSettingsDialog);
    connect(tunePage_, &TunePage::switchStateChanged, context_->deviceAdapter.get(), &IDeviceAdapter::setSwitchState);
    connect(tunePage_, &TunePage::filamentSwitchRequested, tunePage_, &TunePage::toggleDetectorMode);
    connect(tunePage_, &TunePage::connectRequested, this, [this](const DeviceConnectionConfig &config) {
        lastConnectionConfig_ = config;
        tunePage_->setConnectionConfig(config);
        settingsPage_->setConnectionConfig(config);
        context_->connectionService->connectToDevice(config);
    });
    connect(tunePage_, &TunePage::disconnectRequested, this, [this]() {
        context_->connectionService->disconnectFromDevice();
    });

    connect(monitorPage_, &MonitorPage::methodSaveRequested, this, [this](const MonitorMethod &method) {
        context_->monitorService->saveMethod(method);
        monitorPage_->setAvailableMethods(context_->monitorService->listMethods());
        statusBar()->showMessage("监测方法已保存", 3000);
    });
    connect(monitorPage_, &MonitorPage::methodLoadRequested, this, [this](const QString &fileName) {
        if (fileName.isEmpty()) {
            return;
        }
        const QString path = AppSettings::monitorMethodDirectory() + "/" + fileName;
        const MonitorMethod method = context_->monitorService->loadMethod(path);
        monitorPage_->applyMethod(method);
    });
    connect(monitorPage_, &MonitorPage::startRequested, this, [this](const MonitorMethod &method) {
        context_->monitorService->applyMethod(method);
        context_->scanControlService->startScan(method.scanSettings);
    });
    connect(monitorPage_, &MonitorPage::stopRequested, context_->scanControlService.get(), &ScanControlService::stopScan);
    connect(dataPage_, &DataPage::openDataRequested, this, &MainWindow::openFrameViewer);
    connect(settingsPage_, &SettingsPage::connectRequested, this, [this](const DeviceConnectionConfig &config) {
        lastConnectionConfig_ = config;
        tunePage_->setConnectionConfig(config);
        settingsPage_->setConnectionConfig(config);
        context_->connectionService->connectToDevice(config);
    });
    connect(settingsPage_, &SettingsPage::disconnectRequested, this, [this]() {
        context_->connectionService->disconnectFromDevice();
    });
}

void MainWindow::updateStatusBar(const InstrumentStatus &status) {
    statusBar()->showMessage(QString("连接: %1 | 真空度: %2 | 扫描: %3")
                                 .arg(status.connected ? "已连接" : "未连接")
                                 .arg(QString::number(status.vacuum.valuePa, 'e', 2))
                                 .arg(status.scanning ? "运行中" : "停止"));
}

void MainWindow::showError(const QString &message) {
    QMessageBox::warning(this, "提示", message);
}

void MainWindow::openConnectionDialog() {
    connectionDialog_->setConfig(context_->connectionService->recentConnection());
    if (connectionDialog_->exec() == QDialog::Accepted) {
        lastConnectionConfig_ = connectionDialog_->config();
        tunePage_->setConnectionConfig(lastConnectionConfig_);
        context_->connectionService->connectToDevice(lastConnectionConfig_);
    }
}

void MainWindow::openScanSettingsDialog() {
    scanSettingsDialog_->setSettings(tunePage_->scanSettings());
    if (scanSettingsDialog_->exec() != QDialog::Accepted) {
        return;
    }
    const ScanSettings settings = scanSettingsDialog_->settings();
    QString errorMessage;
    if (!settings.isValid(&errorMessage)) {
        showError(errorMessage);
        return;
    }
    tunePage_->applyScanSettings(settings);
    statusBar()->showMessage(QString("扫描设置已更新: %1").arg(scanModeToString(settings.mode)), 3000);
}

void MainWindow::openFrameViewer() {
    const QString filePath = QFileDialog::getOpenFileName(this, "打开单帧数据", AppSettings::frameDirectory(), "JSON Files (*.json)");
    if (filePath.isEmpty()) {
        return;
    }
    const SpectrumFrame frame = applyDataProcessing(context_->persistenceService->loadFrame(filePath),
                                                    context_->settingsService->dataProcessingSettings());
    dataPage_->setFrame(frame);
    navigation_->setCurrentRow(3);
    FrameViewerDialog dialog(this);
    dialog.setChartSettings(context_->settingsService->chartSettings());
    dialog.setFrame(frame);
    dialog.exec();
}

void MainWindow::openInstrumentControlDialog() {
    instrumentControlDialog_->setSettings(context_->settingsService->instrumentControlSettings());
    if (instrumentControlDialog_->exec() != QDialog::Accepted) {
        return;
    }
    const auto settings = instrumentControlDialog_->settings();
    context_->settingsService->saveInstrumentControlSettings(settings);
    context_->deviceAdapter->setSwitchState(InstrumentSwitch::ForePump, settings.forePumpEnabled);
    context_->deviceAdapter->setSwitchState(InstrumentSwitch::ForeValve, settings.foreValveEnabled);
    context_->deviceAdapter->setSwitchState(InstrumentSwitch::MolecularPump, settings.molecularPumpEnabled);
    context_->deviceAdapter->setSwitchState(InstrumentSwitch::InletValve, settings.inletValveEnabled);
    context_->deviceAdapter->setSwitchState(InstrumentSwitch::Filament, settings.filamentEnabled);
    context_->deviceAdapter->setSwitchState(InstrumentSwitch::Multiplier, settings.multiplierEnabled);
    statusBar()->showMessage("仪器控制设置已应用", 3000);
}

void MainWindow::openDataProcessingDialog() {
    dataProcessingDialog_->setSettings(context_->settingsService->dataProcessingSettings());
    if (dataProcessingDialog_->exec() != QDialog::Accepted) {
        return;
    }
    const auto settings = dataProcessingDialog_->settings();
    context_->settingsService->saveDataProcessingSettings(settings);
    context_->deviceAdapter->applyDataProcessingSettings(settings);
    statusBar()->showMessage("数据处理参数已保存", 3000);
}

void MainWindow::openSystemSettingsDialog() {
    systemSettingsDialog_->setSettings(context_->settingsService->systemSettings());
    if (systemSettingsDialog_->exec() != QDialog::Accepted) {
        return;
    }
    const auto settings = systemSettingsDialog_->settings();
    context_->settingsService->saveSystemSettings(settings);
    applySystemSettings(settings);
    statusBar()->showMessage("系统设置已保存", 3000);
}

void MainWindow::openChartSettingsDialog() {
    chartSettingsDialog_->setSettings(context_->settingsService->chartSettings());
    if (chartSettingsDialog_->exec() != QDialog::Accepted) {
        return;
    }
    const auto settings = chartSettingsDialog_->settings();
    context_->settingsService->saveChartSettings(settings);
    tunePage_->applyChartSettings(settings);
    monitorPage_->applyChartSettings(settings);
    dataPage_->applyChartSettings(settings);
    statusBar()->showMessage("谱图设置已应用", 3000);
}

void MainWindow::syncToolbarSwitches(const InstrumentStatus &status) {
    for (auto it = switchActions_.cbegin(); it != switchActions_.cend(); ++it) {
        QAction *action = it.value();
        const bool checked = status.switchStates.value(it.key(), false);
        action->blockSignals(true);
        action->setChecked(checked);
        action->setEnabled(status.connected);
        action->blockSignals(false);
    }
}

void MainWindow::applySystemSettings(const SystemSettings &settings) {
    Q_UNUSED(settings);
    if (statusRefreshTimer_) {
        statusRefreshTimer_->setInterval(kStatusRefreshIntervalMs);
        statusRefreshTimer_->start();
    }
}

void MainWindow::restoreWindowStateFromSettings() {
    if (!kRememberWindowLayout) {
        return;
    }
    QSettings qtSettings(QApplication::organizationName(), QApplication::applicationName());
    restoreGeometry(qtSettings.value("window/geometry").toByteArray());
    const int pageIndex = qtSettings.value("window/pageIndex", 0).toInt();
    navigation_->setCurrentRow(qBound(0, pageIndex, stack_->count() - 1));
}

void MainWindow::persistWindowStateToSettings() {
    if (!kRememberWindowLayout) {
        return;
    }
    QSettings qtSettings(QApplication::organizationName(), QApplication::applicationName());
    qtSettings.setValue("window/geometry", saveGeometry());
    qtSettings.setValue("window/pageIndex", navigation_->currentRow());
}

void MainWindow::updateActionStates(const InstrumentStatus &status) {
    const bool connected = status.connected;
    const bool scanning = status.scanning;
    if (startScanAction_) {
        startScanAction_->setEnabled(connected && !scanning);
    }
    if (stopScanAction_) {
        stopScanAction_->setEnabled(connected && scanning);
    }
    if (calibrateAction_) {
        calibrateAction_->setEnabled(connected && !scanning);
    }
    if (saveFrameAction_) {
        saveFrameAction_->setEnabled(lastFrame_.timestamp.isValid());
    }
    if (disconnectAction_) {
        disconnectAction_->setEnabled(connected && !scanning);
    }
    if (openConnectionAction_) {
        openConnectionAction_->setEnabled(!scanning);
    }
    if (openInstrumentControlAction_) {
        openInstrumentControlAction_->setEnabled(!scanning);
    }
    if (openScanSettingsAction_) {
        openScanSettingsAction_->setEnabled(!scanning);
    }
    if (openDataProcessingAction_) {
        openDataProcessingAction_->setEnabled(!scanning);
    }
    if (openSystemSettingsAction_) {
        openSystemSettingsAction_->setEnabled(!scanning);
    }
    if (openChartSettingsAction_) {
        openChartSettingsAction_->setEnabled(true);
    }
}

void MainWindow::showMenuBelowButton(QMenu *menu, QWidget *button) {
    if (!menu || !button) {
        return;
    }
    menu->exec(button->mapToGlobal(QPoint(0, button->height())));
}

void MainWindow::closeEvent(QCloseEvent *event) {
    persistWindowStateToSettings();
    QMainWindow::closeEvent(event);
}

}  // namespace deviceapp
