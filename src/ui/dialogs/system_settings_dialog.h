#pragma once

#include <QDialog>

#include "core/types.h"

class QCheckBox;
class QComboBox;
class QLineEdit;
class QPushButton;

namespace deviceapp {

class SystemSettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SystemSettingsDialog(QWidget *parent = nullptr);

    void setSettings(const SystemSettings &settings);
    SystemSettings settings() const;

private:
    QLineEdit *rfddsEdit_;
    QPushButton *applyRfddsButton_;
    QLineEdit *serialNumberEdit_;
    QComboBox *modelBox_;
    QPushButton *applySystemInfoButton_;
};

}  // namespace deviceapp
