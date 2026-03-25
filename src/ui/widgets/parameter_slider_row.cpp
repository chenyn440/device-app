#include "ui/widgets/parameter_slider_row.h"

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>

namespace deviceapp {

ParameterSliderRow::ParameterSliderRow(const QString &label, const QString &unit, QWidget *parent) : QWidget(parent) {
    label_ = new QLabel(label, this);
    label_->setMinimumWidth(92);
    spinBox_ = new QDoubleSpinBox(this);
    spinBox_->setButtonSymbols(QAbstractSpinBox::NoButtons);
    spinBox_->setDecimals(1);
    spinBox_->setRange(minimum_, maximum_);
    unitLabel_ = new QLabel(unit, this);
    unitLabel_->setMinimumWidth(36);
    unitLabel_->setAlignment(Qt::AlignCenter);
    slider_ = new QSlider(Qt::Horizontal, this);
    slider_->setRange(0, toSliderValue(maximum_));

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    layout->addWidget(label_);
    layout->addWidget(spinBox_);
    layout->addWidget(unitLabel_);
    layout->addWidget(slider_, 1);

    connect(spinBox_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        slider_->blockSignals(true);
        slider_->setValue(toSliderValue(value));
        slider_->blockSignals(false);
        emit valueChanged(value);
    });
    connect(slider_, &QSlider::valueChanged, this, [this](int value) {
        const double mapped = fromSliderValue(value);
        spinBox_->blockSignals(true);
        spinBox_->setValue(mapped);
        spinBox_->blockSignals(false);
        emit valueChanged(mapped);
    });
}

void ParameterSliderRow::setRange(double minimum, double maximum) {
    minimum_ = minimum;
    maximum_ = maximum;
    spinBox_->setRange(minimum_, maximum_);
    slider_->setRange(0, toSliderValue(maximum_));
}

void ParameterSliderRow::setValue(double value) {
    spinBox_->setValue(value);
}

double ParameterSliderRow::value() const {
    return spinBox_->value();
}

void ParameterSliderRow::setSingleStep(double step) {
    spinBox_->setSingleStep(step);
}

void ParameterSliderRow::setDecimals(int decimals) {
    spinBox_->setDecimals(decimals);
    scale_ = 1.0;
    for (int i = 0; i < decimals + 1; ++i) {
        scale_ *= 10.0;
    }
    slider_->setRange(0, toSliderValue(maximum_));
}

void ParameterSliderRow::setLabelWidth(int width) {
    label_->setMinimumWidth(width);
    label_->setMaximumWidth(width);
}

void ParameterSliderRow::setCompact(bool compact) {
    spinBox_->setFixedWidth(compact ? 72 : 90);
    unitLabel_->setFixedWidth(compact ? 34 : 40);
    layout()->setContentsMargins(0, compact ? 1 : 0, 0, compact ? 1 : 0);
}

int ParameterSliderRow::toSliderValue(double value) const {
    return static_cast<int>((value - minimum_) * scale_);
}

double ParameterSliderRow::fromSliderValue(int value) const {
    return minimum_ + static_cast<double>(value) / scale_;
}

}  // namespace deviceapp
