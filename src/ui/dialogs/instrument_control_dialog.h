#pragma once

#include <QDialog>

#include "core/types.h"

class QCheckBox;
class QDoubleSpinBox;

namespace deviceapp {

class InstrumentControlDialog : public QDialog {
    Q_OBJECT

public:
    explicit InstrumentControlDialog(QWidget *parent = nullptr);

    void setSettings(const InstrumentControlSettings &settings);
    InstrumentControlSettings settings() const;

private:
    QCheckBox *forePumpCheck_;
    QCheckBox *foreValveCheck_;
    QCheckBox *molecularPumpCheck_;
    QCheckBox *inletValveCheck_;
    QCheckBox *filamentCheck_;
    QCheckBox *multiplierCheck_;
    QDoubleSpinBox *sourceTempSpin_;
    QDoubleSpinBox *chamberTempSpin_;
};

}  // namespace deviceapp
