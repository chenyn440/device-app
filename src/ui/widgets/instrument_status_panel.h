#pragma once

#include <QList>
#include <QStringList>
#include <QWidget>

#include "core/types.h"

class QLabel;
class QProgressBar;
class QTableWidget;

namespace deviceapp {

class InstrumentStatusPanel : public QWidget {
    Q_OBJECT

public:
    explicit InstrumentStatusPanel(QWidget *parent = nullptr);

    void setInstrumentStatus(const InstrumentStatus &status, const QStringList &componentValues);

private:
    QList<QLabel *> temperatureTopLabels_;
    QList<QProgressBar *> temperatureBars_;
    QTableWidget *componentTable_;
};

}  // namespace deviceapp
