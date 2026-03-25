#pragma once

#include <QDialog>

#include "core/types.h"

class QLabel;

namespace deviceapp {

class SpectrumView;

class FrameViewerDialog : public QDialog {
    Q_OBJECT

public:
    explicit FrameViewerDialog(QWidget *parent = nullptr);

    void setFrame(const SpectrumFrame &frame);
    void setChartSettings(const ChartSettings &settings);

private:
    QLabel *summaryLabel_;
    SpectrumView *spectrumView_;
};

}  // namespace deviceapp
