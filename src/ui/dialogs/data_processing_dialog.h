#pragma once

#include <QDialog>

#include "core/types.h"

class QCheckBox;
class QDoubleSpinBox;
class QSpinBox;

namespace deviceapp {

class DataProcessingDialog : public QDialog {
    Q_OBJECT

public:
    explicit DataProcessingDialog(QWidget *parent = nullptr);

    void setSettings(const DataProcessingSettings &settings);
    DataProcessingSettings settings() const;

private:
    QCheckBox *smoothingCheck_;
    QSpinBox *windowSpin_;
    QDoubleSpinBox *lambdaSpin_;
    QDoubleSpinBox *asymmetrySpin_;
};

}  // namespace deviceapp
