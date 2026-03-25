#include "ui/dialogs/frame_viewer_dialog.h"

#include <QLabel>
#include <QVBoxLayout>

#include "ui/widgets/spectrum_view.h"

namespace deviceapp {

FrameViewerDialog::FrameViewerDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("单帧数据");
    resize(900, 600);

    auto *layout = new QVBoxLayout(this);
    summaryLabel_ = new QLabel(this);
    spectrumView_ = new SpectrumView(this);
    layout->addWidget(summaryLabel_);
    layout->addWidget(spectrumView_, 1);
}

void FrameViewerDialog::setFrame(const SpectrumFrame &frame) {
    summaryLabel_->setText(QString("时间: %1 | 模式: %2 | 检测器: %3")
                               .arg(frame.timestamp.toString("yyyy-MM-dd HH:mm:ss"))
                               .arg(scanModeToString(frame.scanMode))
                               .arg(detectorTypeToString(frame.detector)));
    spectrumView_->setFrame(frame);
}

void FrameViewerDialog::setChartSettings(const ChartSettings &settings) {
    spectrumView_->applyChartSettings(settings);
}

}  // namespace deviceapp
