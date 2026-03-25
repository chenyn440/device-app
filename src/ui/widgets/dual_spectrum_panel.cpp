#include "ui/widgets/dual_spectrum_panel.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

#include "ui/widgets/spectrum_view.h"

namespace deviceapp {

DualSpectrumPanel::DualSpectrumPanel(QWidget *parent) : QWidget(parent) {
    spectrumView_ = new SpectrumView(this);
    trendView_ = new SpectrumView(this);
    spectrumView_->setTitle("全扫描");
    spectrumView_->setAxisTitles("质量数 MZ", "强度");
    spectrumView_->setAxisRanges(0.0, 1000.0, 0.0, 1000.0);
    spectrumView_->setGridVisible(false);

    trendView_->setTitle("TIC");
    trendView_->setAxisTitles("s", "强度");
    trendView_->setAxisRanges(0.0, 1000.0, 0.0, 1000.0);
    trendView_->setGridVisible(false);

    auto *pagerRow = new QWidget(this);
    auto *pagerLayout = new QHBoxLayout(pagerRow);
    pagerLayout->setContentsMargins(0, 0, 0, 0);
    pagerLayout->setSpacing(2);
    prevButton_ = new QPushButton("|◀", this);
    nextButton_ = new QPushButton("▶|", this);
    prevButton_->setFixedSize(24, 24);
    nextButton_->setFixedSize(24, 24);
    prevButton_->setStyleSheet("QPushButton { background: white; border: 1px solid #8f98a3; padding: 0; }");
    nextButton_->setStyleSheet("QPushButton { background: white; border: 1px solid #8f98a3; padding: 0; }");
    pagerLayout->addWidget(prevButton_);
    pagerLayout->addWidget(nextButton_);
    pagerLayout->addStretch();

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(3);
    layout->addWidget(spectrumView_, 5);
    layout->addWidget(pagerRow, 0);
    layout->addWidget(trendView_, 2);
}

void DualSpectrumPanel::setSpectrumFrame(const SpectrumFrame &frame) {
    spectrumView_->setFrame(frame);
    trendView_->setFrame(buildTrendFrame(frame));
}

void DualSpectrumPanel::applyChartSettings(const ChartSettings &settings) {
    ChartSettings mainSettings = settings;
    mainSettings.xAxisMode = "mass";
    mainSettings.fixedYRange = true;
    mainSettings.yMin = 0.0;
    mainSettings.yMax = 1000.0;
    spectrumView_->applyChartSettings(mainSettings);
    spectrumView_->setAxisTitles("质量数 MZ", "强度");
    spectrumView_->setAxisRanges(0.0, 1000.0, 0.0, 1000.0);
    spectrumView_->setGridVisible(false);

    ChartSettings trendSettings = settings;
    trendSettings.xAxisMode = "time";
    trendSettings.fixedYRange = true;
    trendSettings.yMin = 0.0;
    trendSettings.yMax = 1000.0;
    trendSettings.peakCount = 0;
    trendView_->applyChartSettings(trendSettings);
    trendView_->setAxisTitles("s", "强度");
    trendView_->setAxisRanges(0.0, 1000.0, 0.0, 1000.0);
    trendView_->setGridVisible(false);
}

void DualSpectrumPanel::setDisplayMode(ScanMode mode) {
    Q_UNUSED(mode);
}

SpectrumFrame DualSpectrumPanel::buildTrendFrame(const SpectrumFrame &frame) {
    SpectrumFrame trend;
    trend.timestamp = frame.timestamp;
    trend.scanMode = ScanMode::FullScan;
    trend.detector = frame.detector;

    const int buckets = 24;
    if (frame.intensities.isEmpty()) {
        trend.masses = {0.0, 1000.0};
        trend.intensities = {0.0, 0.0};
        return trend;
    }

    const int bucketSize = qMax(1, frame.intensities.size() / buckets);
    for (int i = 0; i < frame.intensities.size(); i += bucketSize) {
        double sum = 0.0;
        int count = 0;
        for (int j = i; j < qMin(i + bucketSize, frame.intensities.size()); ++j) {
            sum += frame.intensities[j];
            ++count;
        }
        trend.masses.append(static_cast<double>(trend.masses.size()) * (1000.0 / buckets));
        trend.intensities.append(count > 0 ? sum / count : 0.0);
    }
    return trend;
}

}  // namespace deviceapp
