#pragma once

#include <QWidget>

#include "core/types.h"

class QPushButton;

namespace deviceapp {

class SpectrumView;

class DualSpectrumPanel : public QWidget {
    Q_OBJECT

public:
    explicit DualSpectrumPanel(QWidget *parent = nullptr);

    void setSpectrumFrame(const SpectrumFrame &frame);
    void applyChartSettings(const ChartSettings &settings);
    void setDisplayMode(ScanMode mode);

private:
    SpectrumFrame buildTrendFrame(const SpectrumFrame &frame);
    SpectrumView *spectrumView_;
    SpectrumView *trendView_;
    QPushButton *prevButton_;
    QPushButton *nextButton_;
};

}  // namespace deviceapp
