#pragma once

#include <QDialog>

#include "core/types.h"

class QCheckBox;
class QDoubleSpinBox;
class QRadioButton;
class QSpinBox;

namespace deviceapp {

class ChartSettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit ChartSettingsDialog(QWidget *parent = nullptr);

    void setSettings(const ChartSettings &settings);
    ChartSettings settings() const;

private:
    void updateYRangeState();

    QRadioButton *xAxisMassRadio_;
    QRadioButton *xAxisVoltageRadio_;
    QRadioButton *xAxisTimeRadio_;
    QRadioButton *xAxisPointRadio_;
    QCheckBox *disableAutoRestoreCheck_;
    QCheckBox *fixedYRangeCheck_;
    QDoubleSpinBox *yMinSpin_;
    QDoubleSpinBox *yMaxSpin_;
    QSpinBox *peakCountSpin_;
    QCheckBox *showRawDataCheck_;
    QCheckBox *showPersistenceCheck_;
    QCheckBox *showHalfPeakWidthCheck_;
    QCheckBox *showTicCheck_;
};

}  // namespace deviceapp
