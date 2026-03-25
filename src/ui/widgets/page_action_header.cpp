#include "ui/widgets/page_action_header.h"

#include <QHBoxLayout>
#include <QPushButton>

namespace deviceapp {

PageActionHeader::PageActionHeader(QWidget *parent) : QWidget(parent) {
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(6, 5, 6, 5);
    layout->setSpacing(4);

    setStyleSheet(
        "PageActionHeader { background: #eef2f6; border: 1px solid #d0d5dd; }"
        "QPushButton { min-height: 16px; max-height: 16px; padding: 1px 8px; border: 1px solid #b6bcc6; border-radius: 3px; background: #f8fafc; font-size: 12px; }"
        "QPushButton:checked { background: #1f8f4c; color: white; border-color: #1f8f4c; }"
        "QPushButton:disabled { background: #eaecf0; color: #98a2b3; border-color: #d0d5dd; }");

    auto *startButton = createActionButton("开始");
    auto *stopButton = createActionButton("停止");
    auto *calibrateButton = createActionButton("校正当前质量轴");
    auto *forePumpButton = createActionButton("前级泵", true);
    auto *foreValveButton = createActionButton("前级阀", true);
    auto *molecularPumpButton = createActionButton("分子泵", true);
    auto *inletValveButton = createActionButton("进样阀", true);
    auto *filamentButton = createActionButton("灯丝", true);
    auto *multiplierButton = createActionButton("倍增器", true);
    auto *filamentSwitchButton = createActionButton("灯丝切换");
    auto *saveButton = createActionButton("数据保存");
    auto *refreshButton = createActionButton("谱图刷新");

    actionButtons_.insert(HeaderAction::Start, startButton);
    actionButtons_.insert(HeaderAction::Stop, stopButton);
    actionButtons_.insert(HeaderAction::Calibrate, calibrateButton);
    actionButtons_.insert(HeaderAction::FilamentSwitch, filamentSwitchButton);
    actionButtons_.insert(HeaderAction::SaveData, saveButton);
    actionButtons_.insert(HeaderAction::RefreshChart, refreshButton);

    switchButtons_.insert(InstrumentSwitch::ForePump, forePumpButton);
    switchButtons_.insert(InstrumentSwitch::ForeValve, foreValveButton);
    switchButtons_.insert(InstrumentSwitch::MolecularPump, molecularPumpButton);
    switchButtons_.insert(InstrumentSwitch::InletValve, inletValveButton);
    switchButtons_.insert(InstrumentSwitch::Filament, filamentButton);
    switchButtons_.insert(InstrumentSwitch::Multiplier, multiplierButton);

    layout->addWidget(startButton);
    layout->addWidget(stopButton);
    layout->addWidget(calibrateButton);
    layout->addWidget(forePumpButton);
    layout->addWidget(foreValveButton);
    layout->addWidget(molecularPumpButton);
    layout->addWidget(inletValveButton);
    layout->addWidget(filamentButton);
    layout->addWidget(multiplierButton);
    layout->addWidget(filamentSwitchButton);
    layout->addWidget(saveButton);
    layout->addWidget(refreshButton);
    layout->addStretch();

    connect(startButton, &QPushButton::clicked, this, &PageActionHeader::startRequested);
    connect(stopButton, &QPushButton::clicked, this, &PageActionHeader::stopRequested);
    connect(calibrateButton, &QPushButton::clicked, this, &PageActionHeader::calibrateRequested);
    connect(filamentSwitchButton, &QPushButton::clicked, this, &PageActionHeader::filamentSwitchRequested);
    connect(saveButton, &QPushButton::clicked, this, &PageActionHeader::saveRequested);
    connect(refreshButton, &QPushButton::clicked, this, &PageActionHeader::refreshRequested);

    for (auto it = switchButtons_.cbegin(); it != switchButtons_.cend(); ++it) {
        const InstrumentSwitch instrumentSwitch = it.key();
        auto *button = it.value();
        connect(button, &QPushButton::toggled, this, [this, instrumentSwitch](bool checked) {
            emit switchStateChanged(instrumentSwitch, checked);
        });
    }
}

void PageActionHeader::setActionEnabled(HeaderAction action, bool enabled) {
    if (actionButtons_.contains(action)) {
        actionButtons_.value(action)->setEnabled(enabled);
    }
}

void PageActionHeader::setActionVisible(HeaderAction action, bool visible) {
    if (actionButtons_.contains(action)) {
        actionButtons_.value(action)->setVisible(visible);
    }
}

void PageActionHeader::setSwitchEnabled(InstrumentSwitch instrumentSwitch, bool enabled) {
    if (switchButtons_.contains(instrumentSwitch)) {
        switchButtons_.value(instrumentSwitch)->setEnabled(enabled);
    }
}

void PageActionHeader::setSwitchState(InstrumentSwitch instrumentSwitch, bool checked) {
    if (!switchButtons_.contains(instrumentSwitch)) {
        return;
    }
    auto *button = switchButtons_.value(instrumentSwitch);
    button->blockSignals(true);
    button->setChecked(checked);
    button->blockSignals(false);
}

QPushButton *PageActionHeader::createActionButton(const QString &text, bool checkable) {
    auto *button = new QPushButton(text, this);
    button->setCheckable(checkable);
    return button;
}

}  // namespace deviceapp
