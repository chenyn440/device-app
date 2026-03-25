#pragma once

#include <QWidget>

#include "core/types.h"

class QLabel;
class QPushButton;

namespace deviceapp {

class SpectrumView;

class DataPage : public QWidget {
    Q_OBJECT

public:
    explicit DataPage(QWidget *parent = nullptr);

    void setFrame(const SpectrumFrame &frame);
    void applyChartSettings(const ChartSettings &settings);
    void clear();

signals:
    void openDataRequested();

private:
    SpectrumFrame buildTrendFrame(const SpectrumFrame &frame) const;

    QPushButton *openDataButton_;
    SpectrumView *mainSpectrumView_;
    SpectrumView *ticSpectrumView_;
    QLabel *configTitleLabel_;
    QLabel *runStatusLabel_;
};

}  // namespace deviceapp
