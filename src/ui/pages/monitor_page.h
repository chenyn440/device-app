#pragma once

#include <QComboBox>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QTableWidget>
#include <QTabWidget>
#include <QWidget>

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include "app/services.h"
#include "ui/widgets/instrument_status_panel.h"
#include "ui/widgets/page_action_header.h"

namespace deviceapp {

class MonitorPage : public QWidget {
    Q_OBJECT

public:
    explicit MonitorPage(QWidget *parent = nullptr);

    MonitorMethod currentMethod() const;
    void applyMethod(const MonitorMethod &method);
    void setFrame(const SpectrumFrame &frame);
    void setAvailableMethods(const QStringList &files);
    void applyChartSettings(const ChartSettings &settings);
    void setScanState(bool connected, bool scanning);
    void setInstrumentStatus(const InstrumentStatus &status);

signals:
    void methodSaveRequested(const MonitorMethod &method);
    void methodLoadRequested(const QString &fileName);
    void startRequested(const MonitorMethod &method);
    void stopRequested();

private:
    void syncModeSections();
    QVector<double> selectedIons() const;
    void rebuildRicChart(const SpectrumFrame &frame);
    void rebuildTicChart(const SpectrumFrame &frame);

    PageActionHeader *actionHeader_;
    QPushButton *alarmModeButton_;
    QPushButton *analysisModeButton_;
    QPushButton *openMethodButton_;
    QPushButton *runMethodButton_;
    QPushButton *pauseRefreshButton_;
    QPushButton *saveDataButton_;
    QLabel *toolbarStatusLabel_;
    QChartView *ricChartView_;
    QChart *ricChart_;
    QValueAxis *ricAxisX_;
    QValueAxis *ricAxisY_;
    QChartView *ticChartView_;
    QChart *ticChart_;
    QValueAxis *ticAxisX_;
    QValueAxis *ticAxisY_;
    QTabWidget *rightTabs_;
    QWidget *parameterTab_;
    InstrumentStatusPanel *statusPanel_;
    QVector<QCheckBox *> ionEnabledChecks_;
    QRadioButton *detectorMultiplierRadio_;
    QRadioButton *detectorFaradayRadio_;
    QRadioButton *stabilityZoneOneRadio_;
    QRadioButton *stabilityZoneTwoRadio_;
    QRadioButton *fullScanRadio_;
    QRadioButton *simRadio_;
    QFrame *simSettingsFrame_;
    QLineEdit *dwellTimeEdit_;
    QLineEdit *flybackTimeEdit_;
    QLineEdit *peakWidthEdit_;
    QLineEdit *rampVoltageEdit_;
    QTableWidget *simTable_;
    QPushButton *deleteSelectedButton_;
    QPushButton *parameterSettingsButton_;
    QLabel *modeHintLabel_;
    QLabel *runStatusLabel_;
    bool displayPaused_ = false;
    SpectrumFrame lastFrame_;
};

}  // namespace deviceapp
