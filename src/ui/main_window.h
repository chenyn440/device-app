#pragma once

#include <QMainWindow>
#include <QByteArray>

#include "app/application_context.h"
#include "core/types.h"

class QListWidget;
class QStackedWidget;
class QAction;
class QTimer;
class QMenu;
class QWidget;
class QToolBar;

namespace deviceapp {

class TunePage;
class MonitorPage;
class DataPage;
class SettingsPage;
class WorkbenchPlaceholderPage;
class ConnectionDialog;
class ScanSettingsDialog;
class FrameViewerDialog;
class InstrumentControlDialog;
class DataProcessingDialog;
class SystemSettingsDialog;
class ChartSettingsDialog;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(ApplicationContext *context, QWidget *parent = nullptr);

private:
    void setupUi();
    void setupMenus();
    void setupToolbar();
    void wireSignals();
    void updateStatusBar(const InstrumentStatus &status);
    void showError(const QString &message);
    void openConnectionDialog();
    void openScanSettingsDialog();
    void openFrameViewer();
    void openInstrumentControlDialog();
    void openDataProcessingDialog();
    void openSystemSettingsDialog();
    void openChartSettingsDialog();
    void syncToolbarSwitches(const InstrumentStatus &status);
    void applySystemSettings(const SystemSettings &settings);
    void restoreWindowStateFromSettings();
    void persistWindowStateToSettings();
    void updateActionStates(const InstrumentStatus &status);
    void showMenuBelowButton(QMenu *menu, QWidget *button);

protected:
    void closeEvent(QCloseEvent *event) override;

    ApplicationContext *context_;
    InstrumentStatus currentStatus_;
    SpectrumFrame lastFrame_;
    QWidget *menuBarContainer_;
    QWidget *rightPane_;
    QListWidget *navigation_;
    QStackedWidget *stack_;
    QToolBar *toolbar_;
    TunePage *tunePage_;
    MonitorPage *monitorPage_;
    WorkbenchPlaceholderPage *methodPage_;
    DataPage *dataPage_;
    SettingsPage *settingsPage_;
    ConnectionDialog *connectionDialog_;
    ScanSettingsDialog *scanSettingsDialog_;
    InstrumentControlDialog *instrumentControlDialog_;
    DataProcessingDialog *dataProcessingDialog_;
    SystemSettingsDialog *systemSettingsDialog_;
    ChartSettingsDialog *chartSettingsDialog_;
    QAction *startScanAction_;
    QAction *stopScanAction_;
    QAction *calibrateAction_;
    QAction *filamentSwitchAction_;
    QAction *saveFrameAction_;
    QAction *disconnectAction_;
    QAction *openConnectionAction_;
    QAction *openInstrumentControlAction_;
    QAction *openScanSettingsAction_;
    QAction *openDataProcessingAction_;
    QAction *openSystemSettingsAction_;
    QAction *openChartSettingsAction_;
    QMenu *fileMenu_;
    QMenu *runMenu_;
    QMenu *settingsMenu_;
    QMenu *viewMenu_;
    QTimer *statusRefreshTimer_;
    DeviceConnectionConfig lastConnectionConfig_;
    QMap<InstrumentSwitch, QAction *> switchActions_;
};

}  // namespace deviceapp
