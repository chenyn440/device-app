#include "ui/pages/workbench_placeholder_page.h"

#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

namespace deviceapp {

WorkbenchPlaceholderPage::WorkbenchPlaceholderPage(const QString &title, const QString &summary, QWidget *parent) : QWidget(parent) {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    actionHeader_ = new PageActionHeader(this);
    actionHeader_->setActionEnabled(HeaderAction::Start, false);
    actionHeader_->setActionEnabled(HeaderAction::Stop, false);
    actionHeader_->setActionEnabled(HeaderAction::Calibrate, false);
    actionHeader_->setActionEnabled(HeaderAction::FilamentSwitch, false);
    actionHeader_->setActionEnabled(HeaderAction::SaveData, false);
    actionHeader_->setActionVisible(HeaderAction::RefreshChart, false);
    actionHeader_->setSwitchEnabled(InstrumentSwitch::ForePump, false);
    actionHeader_->setSwitchEnabled(InstrumentSwitch::ForeValve, false);
    actionHeader_->setSwitchEnabled(InstrumentSwitch::MolecularPump, false);
    actionHeader_->setSwitchEnabled(InstrumentSwitch::InletValve, false);
    actionHeader_->setSwitchEnabled(InstrumentSwitch::Filament, false);
    actionHeader_->setSwitchEnabled(InstrumentSwitch::Multiplier, false);
    layout->addWidget(actionHeader_);

    auto *card = new QFrame(this);
    card->setFrameShape(QFrame::StyledPanel);
    card->setStyleSheet("QFrame { background: #f8fafc; border: 1px solid #d0d5dd; }");

    auto *cardLayout = new QVBoxLayout(card);
    auto *titleLabel = new QLabel(title, this);
    titleLabel->setStyleSheet("QLabel { font-size: 22px; font-weight: 700; color: #101828; }");
    auto *summaryLabel = new QLabel(summary, this);
    summaryLabel->setWordWrap(true);
    summaryLabel->setStyleSheet("QLabel { color: #475467; font-size: 14px; }");
    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(summaryLabel);
    cardLayout->addStretch();

    layout->addWidget(card);
}

}  // namespace deviceapp
