#pragma once

#include <QWidget>

#include "ui/widgets/page_action_header.h"

namespace deviceapp {

class WorkbenchPlaceholderPage : public QWidget {
    Q_OBJECT

public:
    WorkbenchPlaceholderPage(const QString &title, const QString &summary, QWidget *parent = nullptr);

private:
    PageActionHeader *actionHeader_;
};

}  // namespace deviceapp
