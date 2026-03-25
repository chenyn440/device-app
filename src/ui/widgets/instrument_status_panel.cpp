#include "ui/widgets/instrument_status_panel.h"

#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace deviceapp {

InstrumentStatusPanel::InstrumentStatusPanel(QWidget *parent) : QWidget(parent) {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(10);

    const QString groupStyle =
        "QGroupBox { font-weight: 600; border: 1px solid #d8dbe2; margin-top: 9px; background: #ffffff; color: #344054; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; color: #475467; }";

    auto *temperatureGroup = new QGroupBox("部件温度[℃]", this);
    temperatureGroup->setStyleSheet(groupStyle);
    auto *temperatureGroupLayout = new QHBoxLayout(temperatureGroup);
    temperatureGroupLayout->setContentsMargins(12, 14, 12, 10);
    temperatureGroupLayout->setSpacing(12);
    const QStringList temperatureLabels = {"进样管路1", "进样管路2", "进样腔体", "真空腔体"};
    for (const QString &labelText : temperatureLabels) {
        auto *column = new QWidget(this);
        auto *columnLayout = new QVBoxLayout(column);
        columnLayout->setContentsMargins(0, 0, 0, 0);
        columnLayout->setSpacing(5);
        auto *topLabel = new QLabel("0", this);
        topLabel->setAlignment(Qt::AlignHCenter);
        topLabel->setStyleSheet("QLabel { color: #344054; font-size: 13px; font-weight: 600; }");
        auto *bar = new QProgressBar(this);
        bar->setOrientation(Qt::Vertical);
        bar->setRange(0, 100);
        bar->setValue(0);
        bar->setTextVisible(false);
        bar->setFixedSize(20, 156);
        bar->setStyleSheet(
            "QProgressBar { border: 1px solid #c8cdd6; border-radius: 3px; background: #f7f8fa; }"
            "QProgressBar::chunk { background: #57a5ff; border-radius: 2px; }");
        auto *bottomLabel = new QLabel("100", this);
        bottomLabel->setAlignment(Qt::AlignHCenter);
        bottomLabel->setStyleSheet("QLabel { color: #98a2b3; font-size: 12px; }");
        auto *nameLabel = new QLabel(labelText, this);
        nameLabel->setAlignment(Qt::AlignHCenter);
        nameLabel->setWordWrap(true);
        nameLabel->setStyleSheet("QLabel { color: #475467; font-size: 12px; }");
        columnLayout->addWidget(topLabel);
        columnLayout->addWidget(bar, 0, Qt::AlignHCenter);
        columnLayout->addWidget(bottomLabel);
        columnLayout->addWidget(nameLabel);
        temperatureTopLabels_.append(topLabel);
        temperatureBars_.append(bar);
        temperatureGroupLayout->addWidget(column);
    }

    auto *stateGroup = new QGroupBox("部件状态", this);
    stateGroup->setStyleSheet(groupStyle);
    auto *stateLayout = new QVBoxLayout(stateGroup);
    stateLayout->setContentsMargins(8, 12, 8, 8);
    componentTable_ = new QTableWidget(12, 2, this);
    componentTable_->horizontalHeader()->setVisible(false);
    componentTable_->verticalHeader()->setVisible(false);
    componentTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    componentTable_->verticalHeader()->setDefaultSectionSize(28);
    componentTable_->setAlternatingRowColors(true);
    componentTable_->setStyleSheet(
        "QTableWidget { border: 1px solid #d8dbe2; gridline-color: #e4e7ec; alternate-background-color: #f8fbff; background: white; color: #344054; }"
        "QTableWidget::item { padding: 4px 6px; }"
        "QTableWidget::item:selected { background: #eaf2ff; color: #175cd3; }");
    componentTable_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    const QStringList statusNames = {
        "+24V", "灯丝电流[A]", "电子能量[eV]", "发射电流[μA]", "射频扫描电压[V]", "射频电流[A]",
        "倍增器电压[V]", "前预四极[V]", "打拿极[V]", "推斥极[V]", "透镜1[V]", "透镜2[V]"
    };
    for (int row = 0; row < statusNames.size(); ++row) {
        componentTable_->setItem(row, 0, new QTableWidgetItem(statusNames[row]));
        componentTable_->setItem(row, 1, new QTableWidgetItem("0"));
        if (auto *nameItem = componentTable_->item(row, 0)) {
            nameItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        }
        if (auto *valueItem = componentTable_->item(row, 1)) {
            valueItem->setTextAlignment(Qt::AlignCenter);
        }
    }
    stateLayout->addWidget(componentTable_);

    layout->addWidget(temperatureGroup);
    layout->addWidget(stateGroup);
    layout->addStretch();
}

void InstrumentStatusPanel::setInstrumentStatus(const InstrumentStatus &status, const QStringList &componentValues) {
    for (int i = 0; i < temperatureBars_.size(); ++i) {
        if (i < status.temperatures.size()) {
            const auto &temperature = status.temperatures[i];
            const bool withinTolerance = qAbs(temperature.current - temperature.target) <= temperature.tolerance;
            temperatureTopLabels_[i]->setText(temperature.connected ? QString::number(temperature.current, 'f', 0) : "--");
            temperatureTopLabels_[i]->setStyleSheet(QString("QLabel { color: %1; font-size: 13px; font-weight: 600; }")
                                                        .arg(!temperature.connected ? "#98a2b3" : (withinTolerance ? "#027a48" : "#b42318")));
            temperatureBars_[i]->setValue(temperature.connected ? qBound(0, static_cast<int>(temperature.current), 100) : 0);
        } else {
            temperatureTopLabels_[i]->setText("--");
            temperatureTopLabels_[i]->setStyleSheet("QLabel { color: #98a2b3; font-size: 13px; font-weight: 600; }");
            temperatureBars_[i]->setValue(0);
        }
    }

    for (int row = 0; row < componentValues.size() && row < componentTable_->rowCount(); ++row) {
        if (componentTable_->item(row, 1)) {
            componentTable_->item(row, 1)->setText(componentValues[row]);
        }
    }
}

}  // namespace deviceapp
