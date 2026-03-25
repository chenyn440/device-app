#include "ui/widgets/spectrum_view.h"

#include <QVBoxLayout>

namespace deviceapp {

namespace {

QString xAxisTitleForMode(const QString &mode) {
    if (mode == "voltage") {
        return "Voltage";
    }
    if (mode == "time") {
        return "Time";
    }
    if (mode == "point") {
        return "Point";
    }
    return "m/z";
}

}

SpectrumView::SpectrumView(QWidget *parent) : QWidget(parent) {
    chart_ = new QChart();
    chart_->legend()->hide();
    chart_->setTitle("实时谱图");

    lineSeries_ = new QLineSeries(this);
    peakSeries_ = new QScatterSeries(this);
    peakSeries_->setMarkerSize(8.0);

    axisX_ = new QValueAxis(this);
    axisY_ = new QValueAxis(this);
    axisX_->setTitleText("m/z");
    axisY_->setTitleText("Intensity");

    chart_->addSeries(lineSeries_);
    chart_->addSeries(peakSeries_);
    chart_->addAxis(axisX_, Qt::AlignBottom);
    chart_->addAxis(axisY_, Qt::AlignLeft);
    lineSeries_->attachAxis(axisX_);
    lineSeries_->attachAxis(axisY_);
    peakSeries_->attachAxis(axisX_);
    peakSeries_->attachAxis(axisY_);

    chartView_ = new QChartView(chart_, this);
    chartView_->setRenderHint(QPainter::Antialiasing);
    chartView_->setRubberBand(QChartView::RectangleRubberBand);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(chartView_);
}

void SpectrumView::setFrame(const SpectrumFrame &frame) {
    lineSeries_->clear();
    peakSeries_->clear();

    double maxIntensity = 1.0;
    for (int i = 0; i < frame.masses.size() && i < frame.intensities.size(); ++i) {
        const double xValue = settings_.xAxisMode == "point" ? static_cast<double>(i)
            : settings_.xAxisMode == "time" ? static_cast<double>(i) * 0.1
            : settings_.xAxisMode == "voltage" ? frame.masses[i] * 0.1
            : frame.masses[i];
        lineSeries_->append(xValue, frame.intensities[i]);
        maxIntensity = qMax(maxIntensity, frame.intensities[i]);
    }
    const int peakLimit = settings_.peakCount <= 0 ? 0 : settings_.peakCount;
    int peakIndex = 0;
    for (const PeakInfo &peak : frame.peaks) {
        if (peakLimit > 0 && peakIndex >= peakLimit) {
            break;
        }
        const double xValue = settings_.xAxisMode == "point" ? static_cast<double>(peakIndex)
            : settings_.xAxisMode == "time" ? static_cast<double>(peakIndex) * 0.1
            : settings_.xAxisMode == "voltage" ? peak.mass * 0.1
            : peak.mass;
        peakSeries_->append(xValue, peak.intensity);
        maxIntensity = qMax(maxIntensity, peak.intensity);
        ++peakIndex;
    }
    if (!lineSeries_->points().isEmpty()) {
        axisX_->setRange(lineSeries_->points().first().x(), lineSeries_->points().last().x());
    } else {
        axisX_->setRange(0.0, 100.0);
    }
    if (settings_.fixedYRange) {
        axisY_->setRange(settings_.yMin, settings_.yMax);
    } else {
        axisY_->setRange(0.0, maxIntensity * 1.1);
    }
}

void SpectrumView::setPeakDetailVisible(bool visible) {
    peakSeries_->setVisible(visible);
}

void SpectrumView::applyChartSettings(const ChartSettings &settings) {
    settings_ = settings;
    lineSeries_->setVisible(settings.showRawData || !settings.showPersistence);
    lineSeries_->setOpacity(settings.showPersistence ? 0.55 : 1.0);
    peakSeries_->setVisible(settings.peakCount > 0);
    peakSeries_->setMarkerSize(settings.showHalfPeakWidth ? 10.0 : 8.0);
    axisX_->setTitleText(xAxisTitleForMode(settings.xAxisMode));
    axisY_->setTitleText("Intensity");
    if (settings.fixedYRange) {
        axisY_->setRange(settings.yMin, settings.yMax);
    }
    chartView_->setRubberBand(settings.disableAutoRestoreFullScale
                                  ? QChartView::NoRubberBand
                                  : QChartView::RectangleRubberBand);
}

void SpectrumView::setTitle(const QString &title) {
    chart_->setTitle(title);
}

void SpectrumView::setAxisTitles(const QString &xTitle, const QString &yTitle) {
    axisX_->setTitleText(xTitle);
    axisY_->setTitleText(yTitle);
}

void SpectrumView::setAxisRanges(double xMin, double xMax, double yMin, double yMax) {
    axisX_->setRange(xMin, xMax);
    axisY_->setRange(yMin, yMax);
}

void SpectrumView::setGridVisible(bool visible) {
    axisX_->setGridLineVisible(visible);
    axisY_->setGridLineVisible(visible);
    axisX_->setMinorGridLineVisible(visible);
    axisY_->setMinorGridLineVisible(visible);
}

void SpectrumView::resetZoom() {
    chart_->zoomReset();
}

}  // namespace deviceapp
