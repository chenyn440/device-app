#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QGridLayout>
#include <QLineEdit>
#include <QList>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QTableWidget>
#include <QTabWidget>
#include <QWidget>

#include "app/services.h"
#include "core/types.h"
#include "ui/widgets/dual_spectrum_panel.h"
#include "ui/widgets/instrument_status_panel.h"
#include "ui/widgets/page_action_header.h"
#include "ui/widgets/parameter_slider_row.h"

namespace deviceapp {

class TunePage : public QWidget {
    Q_OBJECT

public:
    TunePage(ScanControlService *scanControlService, TuneService *tuneService, QWidget *parent = nullptr);

    ScanSettings scanSettings() const;
    void applyScanSettings(const ScanSettings &settings);
    TuneParameters tuneParameters() const;
    void setInstrumentStatus(const InstrumentStatus &status);
    void setConnectionConfig(const DeviceConnectionConfig &config);
    void setFrame(const SpectrumFrame &frame);
    void applyChartSettings(const ChartSettings &settings);
    void setScanState(bool connected, bool scanning);
    void toggleDetectorMode();

signals:
    void startRequested(const ScanSettings &settings, const TuneParameters &parameters);
    void stopRequested();
    void saveRequested();
    void refreshRequested();
    void calibrationRequested();
    void scanSettingsRequested();
    void switchStateChanged(InstrumentSwitch instrumentSwitch, bool checked);
    void filamentSwitchRequested();
    void connectRequested(const DeviceConnectionConfig &config);
    void disconnectRequested();

private:
    void syncTargetIonSelection();

    ScanControlService *scanControlService_;
    TuneService *tuneService_;
    PageActionHeader *actionHeader_;
    DualSpectrumPanel *dualSpectrumPanel_;
    QComboBox *scanModeBox_;
    QComboBox *detectorBox_;
    QDoubleSpinBox *massStartSpin_;
    QDoubleSpinBox *massEndSpin_;
    QDoubleSpinBox *resolutionSpin_;
    QDoubleSpinBox *scanTimeSpin_;
    QDoubleSpinBox *sampleRateSpin_;
    QLineEdit *targetIonsEdit_;
    QCheckBox *fullScanCheck_;
    QCheckBox *simCheck_;
    QCheckBox *detectorMultiplierCheck_;
    QCheckBox *detectorFaradayCheck_;
    QCheckBox *stableZoneOneCheck_;
    QCheckBox *stableZoneTwoCheck_;
    QPushButton *scanSettingsButton_;
    QWidget *targetIonBox_;
    QList<QCheckBox *> targetIonChecks_;
    QCheckBox *showAdvancedParametersCheck_;
    ParameterSliderRow *repellerVoltageRow_;
    ParameterSliderRow *lens1VoltageRow_;
    ParameterSliderRow *lens2VoltageRow_;
    ParameterSliderRow *highMassCompensationRow_;
    ParameterSliderRow *lowMassCompensationRow_;
    ParameterSliderRow *multiplierVoltageRow_;
    ParameterSliderRow *rodVoltageRow_;
    ParameterSliderRow *eVoltageRow_;
    ParameterSliderRow *electronEnergyRow_;
    ParameterSliderRow *filamentCurrentRow_;
    ParameterSliderRow *outerDeflectionLensVoltageRow_;
    ParameterSliderRow *innerDeflectionLensVoltageRow_;
    ParameterSliderRow *preQuadrupoleFrontVoltageRow_;
    ParameterSliderRow *preQuadrupoleRearVoltageRow_;
    QWidget *advancedParameterContainer_;
    QPushButton *applyParametersButton_;
    QLabel *modeHintLabel_;
    QLabel *connectionIpValueLabel_;
    QLabel *connectionPortValueLabel_;
    QLabel *connectionStateValueLabel_;
    QPushButton *connectDeviceButton_;
    QPushButton *disconnectDeviceButton_;
    QTableWidget *summaryTable_;
    InstrumentStatusPanel *statusPanel_;
    QTabWidget *rightTabWidget_;
    ScanSettings scanSettingsCache_;
};

}  // namespace deviceapp
