#pragma once

#include <QWidget>

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>

#include "core/types.h"

QT_USE_NAMESPACE

namespace deviceapp {

class SpectrumView : public QWidget {
    Q_OBJECT

public:
    explicit SpectrumView(QWidget *parent = nullptr);

    void setFrame(const SpectrumFrame &frame);
    void setPeakDetailVisible(bool visible);
    void applyChartSettings(const ChartSettings &settings);
    void setTitle(const QString &title);
    void setAxisTitles(const QString &xTitle, const QString &yTitle);
    void setAxisRanges(double xMin, double xMax, double yMin, double yMax);
    void setGridVisible(bool visible);
    void resetZoom();

private:
    ChartSettings settings_;
    QChart *chart_;
    QChartView *chartView_;
    QLineSeries *lineSeries_;
    QScatterSeries *peakSeries_;
    QValueAxis *axisX_;
    QValueAxis *axisY_;
};

}  // namespace deviceapp
