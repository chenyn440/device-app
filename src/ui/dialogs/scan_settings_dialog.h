#pragma once

#include <QDialog>

#include "core/types.h"

class QCheckBox;
class QDoubleSpinBox;
class QComboBox;
class QDialogButtonBox;
class QGroupBox;
class QLineEdit;
class QRadioButton;
class QPushButton;
class QSpinBox;
class QTabWidget;
class QLabel;

namespace deviceapp {

class ScanSettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit ScanSettingsDialog(QWidget *parent = nullptr);

    void setSettings(const ScanSettings &settings);
    ScanSettings settings() const;

private:
    void setVoltageInputsEnabled(bool enabled);
    void updateModeUi();
    void applyMassAxisShortcut();
    void calculateFullScanVoltages();
    void calculateSelectedIonValues();
    void openRfadSettingsDialog();
    QVector<double> selectedTargetIons() const;
    QVector<double> collectTargetVoltages() const;
    void setSelectedTargetIons(const QVector<double> &ions);
    void setTargetVoltages(const QVector<double> &voltages);
    void refreshMassAxisSummary();

    QTabWidget *tabWidget_;
    QRadioButton *fullScanButton_;
    QRadioButton *simButton_;
    QGroupBox *massAxisShortcutGroup_;
    QLabel *shortcutSlopeValueLabel_;
    QLabel *shortcutOffsetValueLabel_;
    QLabel *shortcutEnabledValueLabel_;
    QPushButton *openMassAxisTabButton_;

    QCheckBox *targetIonChecks_[6];
    QDoubleSpinBox *targetVoltageSpins_[6];

    QDoubleSpinBox *startSpin_;
    QDoubleSpinBox *endSpin_;
    QDoubleSpinBox *scanSpeedSpin_;
    QComboBox *scanTimePresetBox_;
    QDoubleSpinBox *customScanTimeSpin_;
    QDoubleSpinBox *flybackSpin_;
    QCheckBox *useTargetVoltageCheck_;
    QDoubleSpinBox *voltageRangeMinSpin_;
    QDoubleSpinBox *voltageRangeMaxSpin_;
    QPushButton *calculateVoltageButton_;
    QGroupBox *fullScanGroup_;
    QGroupBox *selectedIonGroup_;
    QDoubleSpinBox *dwellTimeSpin_;
    QDoubleSpinBox *selectedIonFlybackSpin_;
    QDoubleSpinBox *selectedIonPeakWidthSpin_;
    QDoubleSpinBox *selectedIonRampVoltageSpin_;
    QPushButton *calculateSelectedIonButton_;
    QComboBox *sampleRateBox_;
    QPushButton *rfadSettingsButton_;

    QCheckBox *useMassAxisCheck_;
    QDoubleSpinBox *massAxisSlopeSpin_;
    QDoubleSpinBox *massAxisOffsetSpin_;
    QSpinBox *samplingPointsSpin_;
    QPushButton *applyMassAxisButton_;

    ScanSettings settings_;
};

}  // namespace deviceapp
