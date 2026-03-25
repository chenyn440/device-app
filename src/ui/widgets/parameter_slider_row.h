#pragma once

#include <QWidget>

class QLabel;
class QSlider;
class QDoubleSpinBox;

namespace deviceapp {

class ParameterSliderRow : public QWidget {
    Q_OBJECT

public:
    explicit ParameterSliderRow(const QString &label, const QString &unit = QString(), QWidget *parent = nullptr);

    void setRange(double minimum, double maximum);
    void setValue(double value);
    double value() const;
    void setSingleStep(double step);
    void setDecimals(int decimals);
    void setLabelWidth(int width);
    void setCompact(bool compact);

signals:
    void valueChanged(double value);

private:
    int toSliderValue(double value) const;
    double fromSliderValue(int value) const;

    QLabel *label_;
    QDoubleSpinBox *spinBox_;
    QLabel *unitLabel_;
    QSlider *slider_;
    double minimum_ = 0.0;
    double maximum_ = 100.0;
    double scale_ = 100.0;
};

}  // namespace deviceapp
