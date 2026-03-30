#pragma once

#include <QMap>
#include <QWidget>

#include "core/types.h"

class QPushButton;

namespace deviceapp {

enum class HeaderAction {
    Start,
    Stop,
    Calibrate,
    FilamentSwitch,
    SaveData,
    RefreshChart,
    PullModel,
    AiSummary,
    Troubleshoot,
    ExportAiReport
};

class PageActionHeader : public QWidget {
    Q_OBJECT

public:
    explicit PageActionHeader(QWidget *parent = nullptr);

    void setActionEnabled(HeaderAction action, bool enabled);
    void setActionVisible(HeaderAction action, bool visible);
    void setSwitchEnabled(InstrumentSwitch instrumentSwitch, bool enabled);
    void setSwitchState(InstrumentSwitch instrumentSwitch, bool checked);

signals:
    void startRequested();
    void stopRequested();
    void calibrateRequested();
    void filamentSwitchRequested();
    void saveRequested();
    void refreshRequested();
    void pullModelRequested();
    void aiSummaryRequested();
    void troubleshootRequested();
    void exportAiReportRequested();
    void switchStateChanged(InstrumentSwitch instrumentSwitch, bool checked);

private:
    QPushButton *createActionButton(const QString &text, bool checkable = false);

    QMap<HeaderAction, QPushButton *> actionButtons_;
    QMap<InstrumentSwitch, QPushButton *> switchButtons_;
};

}  // namespace deviceapp
