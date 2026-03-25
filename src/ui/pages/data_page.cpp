#include "ui/pages/data_page.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "ui/widgets/spectrum_view.h"

namespace deviceapp {

DataPage::DataPage(QWidget *parent) : QWidget(parent) {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *toolbar = new QFrame(this);
    toolbar->setStyleSheet("QFrame { background: #c8c3e8; border-top: 1px solid #b4aed9; border-bottom: 1px solid #b4aed9; }");
    auto *toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(10, 4, 10, 4);
    toolbarLayout->setSpacing(8);
    openDataButton_ = new QPushButton("打开数据", this);
    openDataButton_->setFixedHeight(28);
    openDataButton_->setStyleSheet(
        "QPushButton { min-width: 96px; border: 1px solid #8c8aac; background: #d7d2ef; color: #2e2a4f; }"
        "QPushButton:pressed { background: #c9c2e8; }");
    toolbarLayout->addWidget(openDataButton_, 0, Qt::AlignLeft);
    toolbarLayout->addStretch();
    mainLayout->addWidget(toolbar);

    auto *content = new QWidget(this);
    auto *contentLayout = new QHBoxLayout(content);
    contentLayout->setContentsMargins(0, 4, 0, 0);
    contentLayout->setSpacing(0);

    auto *chartContainer = new QFrame(this);
    chartContainer->setStyleSheet("QFrame { background: #f7f7f7; border-left: 1px solid #a9adb7; border-right: 1px solid #a9adb7; border-bottom: 1px solid #a9adb7; }");
    auto *chartLayout = new QVBoxLayout(chartContainer);
    chartLayout->setContentsMargins(2, 2, 2, 2);
    chartLayout->setSpacing(2);
    mainSpectrumView_ = new SpectrumView(this);
    mainSpectrumView_->setTitle("全扫描");
    ticSpectrumView_ = new SpectrumView(this);
    ticSpectrumView_->setTitle("TIC");
    chartLayout->addWidget(mainSpectrumView_, 5);
    chartLayout->addWidget(ticSpectrumView_, 2);

    auto *configPanel = new QFrame(this);
    configPanel->setMinimumWidth(258);
    configPanel->setMaximumWidth(258);
    configPanel->setStyleSheet("QFrame { background: white; border-top: 1px solid #a9adb7; border-right: 1px solid #a9adb7; border-bottom: 1px solid #a9adb7; }");
    auto *configLayout = new QVBoxLayout(configPanel);
    configLayout->setContentsMargins(8, 0, 8, 8);
    configLayout->setSpacing(0);
    configTitleLabel_ = new QLabel("配置信息", this);
    configTitleLabel_->setFixedHeight(30);
    configTitleLabel_->setStyleSheet("QLabel { font-weight: 600; color: #4a4a4a; border-bottom: 1px solid #e4e7ec; padding-left: 2px; }");
    configLayout->addWidget(configTitleLabel_);
    configLayout->addStretch();

    contentLayout->addWidget(chartContainer, 1);
    contentLayout->addWidget(configPanel);
    mainLayout->addWidget(content, 1);

    auto *statusBar = new QFrame(this);
    statusBar->setStyleSheet("QFrame { background: #f6f3d7; border: 1px solid #c7c19a; }");
    auto *statusLayout = new QHBoxLayout(statusBar);
    statusLayout->setContentsMargins(4, 2, 4, 2);
    runStatusLabel_ = new QLabel("运行状态", this);
    runStatusLabel_->setStyleSheet("QLabel { color: #4a4a4a; }");
    statusLayout->addWidget(runStatusLabel_, 0, Qt::AlignLeft);
    statusLayout->addStretch();
    mainLayout->addWidget(statusBar);

    connect(openDataButton_, &QPushButton::clicked, this, &DataPage::openDataRequested);

    ChartSettings defaults;
    defaults.showRawData = true;
    defaults.showTicChart = true;
    applyChartSettings(defaults);
    clear();
}

void DataPage::setFrame(const SpectrumFrame &frame) {
    mainSpectrumView_->setFrame(frame);
    ticSpectrumView_->setFrame(buildTrendFrame(frame));
    runStatusLabel_->setText(frame.timestamp.isValid()
                                 ? QString("运行状态    已加载 %1").arg(frame.timestamp.toString("yyyy-MM-dd HH:mm:ss"))
                                 : "运行状态");
}

void DataPage::applyChartSettings(const ChartSettings &settings) {
    ChartSettings mainSettings = settings;
    mainSettings.showRawData = true;
    mainSpectrumView_->applyChartSettings(mainSettings);

    ChartSettings trendSettings = settings;
    trendSettings.xAxisMode = "time";
    trendSettings.showRawData = true;
    trendSettings.peakCount = 0;
    ticSpectrumView_->applyChartSettings(trendSettings);
}

void DataPage::clear() {
    SpectrumFrame emptyMain;
    emptyMain.scanMode = ScanMode::FullScan;
    emptyMain.detector = DetectorType::ElectronMultiplier;
    emptyMain.masses = {0.0, 620.0};
    emptyMain.intensities = {0.0, 0.0};
    mainSpectrumView_->setFrame(emptyMain);

    SpectrumFrame emptyTrend;
    emptyTrend.scanMode = ScanMode::FullScan;
    emptyTrend.detector = DetectorType::ElectronMultiplier;
    emptyTrend.masses = {0.0, 1000.0};
    emptyTrend.intensities = {0.0, 0.0};
    ticSpectrumView_->setFrame(emptyTrend);
    runStatusLabel_->setText("运行状态");
}

SpectrumFrame DataPage::buildTrendFrame(const SpectrumFrame &frame) const {
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
