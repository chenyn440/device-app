#include "ui/pages/settings_page.h"

#include <QCheckBox>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QResizeEvent>
#include <QSettings>
#include <QVBoxLayout>

#include "core/app_settings.h"

namespace deviceapp {

namespace {

constexpr int kCompactBreakpoint = 860;

const QString kFrameStyle = "QFrame { border: 1px solid #bcbcbc; background: #ffffff; }";
const QString kTitleStyle = "QLabel { font-weight: 600; color: #222222; }";
const QString kFieldLabelStyle = "QLabel { color: #222222; border: none; background: transparent; padding: 0; margin: 0; }";
const QString kSectionTagStyle =
    "QLabel { color: #222222; border: none; background: transparent; padding: 0; margin: 0; font-weight: 600; }";
const QString kButtonStyle =
    "QPushButton { min-height: 26px; padding: 0 12px; background: #f7f7f7; border: 1px solid #a9a9a9; border-radius: 4px; color: #222222; }"
    "QPushButton:hover { background: #f1f1f1; }"
    "QPushButton:pressed { background: #e6e6e6; }"
    "QPushButton:disabled { color: #8c8c8c; background: #f3f3f3; border-color: #c6c6c6; }";
const QString kPrimaryButtonStyle =
    "QPushButton { min-height: 26px; padding: 0 12px; background: #eef4fb; border: 1px solid #8aa4c2; border-radius: 4px; color: #163a63; }"
    "QPushButton:hover { background: #e4eef9; }"
    "QPushButton:pressed { background: #d7e6f6; }"
    "QPushButton:disabled { color: #8c8c8c; background: #f3f3f3; border-color: #c6c6c6; }";
const QString kLineEditStyle = "QLineEdit { min-height: 22px; border: 1px solid #a9a9a9; background: #ffffff; }";
const QString kPlainEditStyle = "QPlainTextEdit { border: 1px solid #a9a9a9; background: #ffffff; }";
const QString kTabButtonBaseStyle =
    "QPushButton { min-height: 28px; padding: 0 16px; color: #333333; border: 1px solid #bcbcbc; border-bottom: none; }"
    "QPushButton:hover { background: #f4f4f4; }"
    "QPushButton:pressed { background: #ececec; }";
const QString kTabButtonActiveStyle =
    "QPushButton { background: #ffffff; font-weight: 600; border-top-left-radius: 4px; border-top-right-radius: 4px; }";
const QString kTabButtonInactiveStyle =
    "QPushButton { background: #e7e7e7; font-weight: 500; border-top-left-radius: 4px; border-top-right-radius: 4px; }";

}

static QString connectionTabStyle(bool active, bool first) {
    return kTabButtonBaseStyle +
           (active ? kTabButtonActiveStyle : kTabButtonInactiveStyle) +
           QString("QPushButton { margin-left: %1px; }").arg(first ? 0 : -1);
}

SettingsPage::SettingsPage(QWidget *parent) : QWidget(parent) {
    buildUi();

    loadLocalSettings();
    showSendCheck_->setChecked(true);
    showReceiveCheck_->setChecked(true);
    localConnectionConfig_.host = "127.0.0.1";
    localConnectionConfig_.port = 9000;
    remoteConnectionConfig_.host = "127.0.0.1";
    remoteConnectionConfig_.port = 9001;
    currentConnectionTabIndex_ = 0;
    syncConnectionFormFromCurrentTab();
    refreshConnectionUiState();
    refreshStorageUiState(false);
    refreshPipeUiState();
    updateResponsiveLayout();

    connect(connectButton_, &QPushButton::clicked, this, [this]() {
        saveCurrentConnectionForm();
        connected_ = false;
        connectionMessage_ = "连接中...";
        refreshConnectionUiState();
        runStatusLabel_->setText("运行状态    正在建立连接");
        DeviceConnectionConfig config;
        config.host = ipEdit_->text().trimmed();
        config.port = static_cast<quint16>(portEdit_->text().trimmed().toUShort());
        emit connectRequested(config);
    });
    connect(disconnectButton_, &QPushButton::clicked, this, [this]() {
        connectionMessage_.clear();
        refreshConnectionUiState();
        emit disconnectRequested();
    });
    connect(browseButton_, &QPushButton::clicked, this, [this]() {
        const QString path = QFileDialog::getExistingDirectory(this, "选择保存目录", pathEdit_->text().trimmed());
        if (!path.isEmpty()) {
            pathEdit_->setText(path);
            storageDirty_ = true;
            refreshStorageUiState(false);
        }
    });
    connect(applyStorageButton_, &QPushButton::clicked, this, [this]() {
        saveLocalSettings();
        storageDirty_ = false;
        refreshStorageUiState(true);
        runStatusLabel_->setText("运行状态    存储设置已保存");
    });
    connect(autoSaveCheck_, &QCheckBox::toggled, this, [this](bool) {
        storageDirty_ = true;
        refreshStorageUiState(false);
    });
    connect(pathEdit_, &QLineEdit::textChanged, this, [this](const QString &) {
        storageDirty_ = true;
        refreshStorageUiState(false);
    });
    connect(prefixEdit_, &QLineEdit::textChanged, this, [this](const QString &) {
        storageDirty_ = true;
        refreshStorageUiState(false);
    });
    connect(sendPipeButton_, &QPushButton::clicked, this, [this]() {
        if (showSendCheck_->isChecked()) {
            appendPipeLog(QString("[发送] %1").arg(pipeInputEdit_->text().trimmed()));
        }
    });
    connect(clearPipeButton_, &QPushButton::clicked, this, [this]() {
        pipeLogEdit_->clear();
        refreshPipeUiState();
    });
    auto switchConnectionTab = [this](int index) {
        if (currentConnectionTabIndex_ == index) {
            return;
        }
        saveCurrentConnectionForm();
        currentConnectionTabIndex_ = index;
        syncConnectionFormFromCurrentTab();
        refreshConnectionUiState();
    };
    connect(localConnectionTabButton_, &QPushButton::clicked, this, [switchConnectionTab]() { switchConnectionTab(0); });
    connect(remoteConnectionTabButton_, &QPushButton::clicked, this, [switchConnectionTab]() { switchConnectionTab(1); });
}

void SettingsPage::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    updateResponsiveLayout();
}

void SettingsPage::buildUi() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *page = new QWidget(this);
    contentLayout_ = new QBoxLayout(QBoxLayout::LeftToRight, page);
    contentLayout_->setContentsMargins(6, 6, 6, 6);
    contentLayout_->setSpacing(10);

    leftColumn_ = new QWidget(page);
    leftColumn_->setMinimumWidth(340);
    leftLayout_ = new QBoxLayout(QBoxLayout::TopToBottom, leftColumn_);
    leftLayout_->setContentsMargins(0, 0, 0, 0);
    leftLayout_->setSpacing(0);

    connectionTabBar_ = new QWidget(this);
    connectionTabBar_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto *tabLayout = new QHBoxLayout(connectionTabBar_);
    tabLayout->setContentsMargins(0, 0, 0, 0);
    tabLayout->setSpacing(0);
    localConnectionTabButton_ = new QPushButton("本地连接", this);
    remoteConnectionTabButton_ = new QPushButton("远程中转服务", this);
    localConnectionTabButton_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    remoteConnectionTabButton_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    tabLayout->addWidget(localConnectionTabButton_, 1);
    tabLayout->addWidget(remoteConnectionTabButton_, 1);
    leftLayout_->addWidget(connectionTabBar_);

    leftLayout_->addWidget(buildConnectionSection());
    leftLayout_->addSpacing(10);
    leftLayout_->addWidget(buildStorageSection());
    leftLayout_->addStretch();

    contentLayout_->addWidget(leftColumn_, 0);
    contentLayout_->addWidget(buildPipeSection(), 1);

    mainLayout->addWidget(page, 1);
    mainLayout->addWidget(buildStatusBar());
}

QWidget *SettingsPage::buildConnectionSection() {
    connectionGroup_ = new QFrame(this);
    connectionGroup_->setStyleSheet(kFrameStyle);
    auto *connectionLayout = new QVBoxLayout(connectionGroup_);
    connectionLayout->setContentsMargins(8, 8, 8, 8);
    connectionLayout->setSpacing(6);

    auto *titleRow = new QWidget(this);
    auto *titleLayout = new QHBoxLayout(titleRow);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(6);
    connectionTitleLabel_ = new QLabel("本地连接", this);
    connectionTitleLabel_->setStyleSheet(kSectionTagStyle);
    connectionStateLabel_ = new QLabel("未连接", this);
    connectionStateLabel_->setStyleSheet(kFieldLabelStyle);
    titleLayout->addWidget(connectionTitleLabel_);
    titleLayout->addStretch();
    titleLayout->addWidget(connectionStateLabel_);
    connectionLayout->addWidget(titleRow);

    auto buildRow = [this](const QString &labelText, QLineEdit **editRef) {
        auto *row = new QWidget(this);
        auto *layout = new QHBoxLayout(row);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(4);
        auto *label = new QLabel(labelText, this);
        label->setStyleSheet(kFieldLabelStyle);
        label->setFixedWidth(78);
        auto *edit = new QLineEdit(this);
        edit->setStyleSheet(kLineEditStyle);
        layout->addWidget(label);
        layout->addWidget(edit, 1);
        *editRef = edit;
        return row;
    };

    connectionLayout->addWidget(buildRow("IP地址", &ipEdit_));
    connectionLayout->addWidget(buildRow("端口号", &portEdit_));

    auto *buttonRow = new QWidget(this);
    auto *buttonLayout = new QHBoxLayout(buttonRow);
    buttonLayout->setContentsMargins(78, 4, 0, 2);
    buttonLayout->setSpacing(10);
    connectButton_ = new QPushButton("建立连接", this);
    disconnectButton_ = new QPushButton("断开连接", this);
    connectButton_->setFixedSize(92, 26);
    disconnectButton_->setFixedSize(92, 26);
    connectButton_->setStyleSheet(kPrimaryButtonStyle);
    disconnectButton_->setStyleSheet(kButtonStyle);
    buttonLayout->addWidget(connectButton_);
    buttonLayout->addWidget(disconnectButton_);
    buttonLayout->addStretch();
    connectionLayout->addWidget(buttonRow);

    return connectionGroup_;
}

QWidget *SettingsPage::buildStorageSection() {
    storageGroup_ = new QFrame(this);
    storageGroup_->setStyleSheet(kFrameStyle);
    auto *storageLayout = new QVBoxLayout(storageGroup_);
    storageLayout->setContentsMargins(8, 8, 8, 8);
    storageLayout->setSpacing(6);

    auto *titleRow = new QWidget(this);
    auto *titleLayout = new QHBoxLayout(titleRow);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(6);
    storageTitleLabel_ = new QLabel("存储", this);
    storageTitleLabel_->setStyleSheet(kSectionTagStyle);
    titleLayout->addWidget(storageTitleLabel_);
    titleLayout->addStretch();
    storageLayout->addWidget(titleRow);

    auto *pathRow = new QWidget(this);
    auto *pathLayout = new QHBoxLayout(pathRow);
    pathLayout->setContentsMargins(0, 0, 0, 0);
    pathLayout->setSpacing(4);
    auto *pathLabel = new QLabel("文件路径", this);
    pathLabel->setStyleSheet(kFieldLabelStyle);
    pathLabel->setFixedWidth(78);
    pathEdit_ = new QLineEdit(this);
    pathEdit_->setStyleSheet(kLineEditStyle);
    browseButton_ = new QPushButton("...", this);
    browseButton_->setFixedWidth(34);
    browseButton_->setMinimumHeight(28);
    browseButton_->setStyleSheet(kButtonStyle);
    pathLayout->addWidget(pathLabel);
    pathLayout->addWidget(pathEdit_, 1);
    pathLayout->addWidget(browseButton_);
    storageLayout->addWidget(pathRow);

    auto *prefixRow = new QWidget(this);
    auto *prefixLayout = new QHBoxLayout(prefixRow);
    prefixLayout->setContentsMargins(0, 0, 0, 0);
    prefixLayout->setSpacing(4);
    auto *prefixLabel = new QLabel("文件名前缀", this);
    prefixLabel->setStyleSheet(kFieldLabelStyle);
    prefixLabel->setFixedWidth(78);
    prefixEdit_ = new QLineEdit(this);
    prefixEdit_->setStyleSheet(kLineEditStyle);
    applyStorageButton_ = new QPushButton("设置", this);
    applyStorageButton_->setFixedWidth(60);
    applyStorageButton_->setMinimumHeight(28);
    applyStorageButton_->setStyleSheet(kButtonStyle);
    prefixLayout->addWidget(prefixLabel);
    prefixLayout->addWidget(prefixEdit_, 1);
    prefixLayout->addWidget(applyStorageButton_);
    storageLayout->addWidget(prefixRow);

    autoSaveCheck_ = new QCheckBox("自动保存", this);
    storageLayout->addWidget(autoSaveCheck_, 0, Qt::AlignLeft);

    return storageGroup_;
}

QWidget *SettingsPage::buildPipeSection() {
    pipeGroup_ = new QFrame(this);
    pipeGroup_->setStyleSheet(kFrameStyle);
    auto *pipeLayout = new QVBoxLayout(pipeGroup_);
    pipeLayout->setContentsMargins(8, 8, 8, 8);
    pipeLayout->setSpacing(6);

    auto *titleRow = new QWidget(this);
    auto *titleLayout = new QHBoxLayout(titleRow);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    auto *pipeTitle = new QLabel("管道服务测试", this);
    pipeTitle->setStyleSheet(kSectionTagStyle);
    titleLayout->addWidget(pipeTitle);
    titleLayout->addStretch();
    pipeLayout->addWidget(titleRow);

    auto *pipeTopRow = new QWidget(this);
    auto *pipeTopLayout = new QHBoxLayout(pipeTopRow);
    pipeTopLayout->setContentsMargins(0, 0, 0, 0);
    pipeTopLayout->setSpacing(5);
    pipeInputEdit_ = new QLineEdit("&&$$:11:2,1;@@", this);
    pipeInputEdit_->setStyleSheet(kLineEditStyle);
    sendPipeButton_ = new QPushButton("发送数据", this);
    showSendCheck_ = new QCheckBox("显示发送", this);
    showReceiveCheck_ = new QCheckBox("显示接收", this);
    clearPipeButton_ = new QPushButton("清空数据", this);
    sendPipeButton_->setFixedSize(76, 26);
    clearPipeButton_->setFixedSize(76, 26);
    sendPipeButton_->setStyleSheet(kButtonStyle);
    clearPipeButton_->setStyleSheet(kButtonStyle);
    pipeTopLayout->addWidget(pipeInputEdit_, 1);
    pipeTopLayout->addWidget(sendPipeButton_);
    pipeTopLayout->addWidget(showSendCheck_);
    pipeTopLayout->addWidget(showReceiveCheck_);
    pipeTopLayout->addWidget(clearPipeButton_);
    pipeLayout->addWidget(pipeTopRow);

    pipeLogEdit_ = new QPlainTextEdit(this);
    pipeLogEdit_->setReadOnly(true);
    pipeLogEdit_->setStyleSheet(kPlainEditStyle);
    pipeLogEdit_->setPlaceholderText("暂无日志。发送管道数据后，日志会显示在这里。");
    pipeLogEdit_->setMinimumSize(420, 250);
    pipeLayout->addWidget(pipeLogEdit_, 1);

    return pipeGroup_;
}

QWidget *SettingsPage::buildStatusBar() {
    auto *statusBar = new QFrame(this);
    statusBar->setStyleSheet("QFrame { background: #f6f3d7; border: 1px solid #c7c19a; }");
    auto *statusLayout = new QHBoxLayout(statusBar);
    statusLayout->setContentsMargins(4, 2, 4, 2);
    runStatusLabel_ = new QLabel("运行状态    未连接", this);
    statusLayout->addWidget(runStatusLabel_, 0, Qt::AlignLeft);
    statusLayout->addStretch();
    return statusBar;
}

void SettingsPage::setConnectionConfig(const DeviceConnectionConfig &config) {
    if (currentConnectionTabIndex_ == 0) {
        localConnectionConfig_ = config;
    } else {
        remoteConnectionConfig_ = config;
    }
    syncConnectionFormFromCurrentTab();
}

void SettingsPage::setConnectionState(bool connected, const QString &message) {
    connected_ = connected;
    connectionMessage_ = message;
    refreshConnectionUiState();
    runStatusLabel_->setText(
        QString("运行状态    %1%2")
            .arg(connected ? "已连接" : "未连接")
            .arg(message.isEmpty() ? QString() : QString("  %1").arg(message)));
}

QString SettingsPage::storageDirectory() const {
    return pathEdit_->text().trimmed();
}

QString SettingsPage::filePrefix() const {
    return prefixEdit_->text().trimmed();
}

bool SettingsPage::autoSaveEnabled() const {
    return autoSaveCheck_->isChecked();
}

void SettingsPage::loadLocalSettings() {
    QSettings settings;
    pathEdit_->blockSignals(true);
    prefixEdit_->blockSignals(true);
    autoSaveCheck_->blockSignals(true);
    pathEdit_->setText(settings.value("settingsPage/dataDirectory", AppSettings::frameDirectory()).toString());
    prefixEdit_->setText(settings.value("settingsPage/filePrefix", "Test").toString());
    autoSaveCheck_->setChecked(settings.value("settingsPage/autoSave", true).toBool());
    pathEdit_->blockSignals(false);
    prefixEdit_->blockSignals(false);
    autoSaveCheck_->blockSignals(false);
    storageDirty_ = false;
}

void SettingsPage::saveLocalSettings() const {
    QSettings settings;
    settings.setValue("settingsPage/dataDirectory", pathEdit_->text().trimmed());
    settings.setValue("settingsPage/filePrefix", prefixEdit_->text().trimmed());
    settings.setValue("settingsPage/autoSave", autoSaveCheck_->isChecked());
}

void SettingsPage::appendPipeLog(const QString &text) {
    if (text.trimmed().isEmpty()) {
        return;
    }
    pipeLogEdit_->appendPlainText(text);
    refreshPipeUiState();
}

void SettingsPage::syncConnectionFormFromCurrentTab() {
    const bool local = currentConnectionTabIndex_ == 0;
    const DeviceConnectionConfig &config = local ? localConnectionConfig_ : remoteConnectionConfig_;
    connectionTitleLabel_->setText(local ? "本地连接" : "远程中转服务");
    ipEdit_->setText(config.host);
    portEdit_->setText(config.port == 0 ? QString() : QString::number(config.port));
    localConnectionTabButton_->setStyleSheet(connectionTabStyle(local, true));
    remoteConnectionTabButton_->setStyleSheet(connectionTabStyle(!local, false));
}

void SettingsPage::saveCurrentConnectionForm() {
    DeviceConnectionConfig config;
    config.host = ipEdit_->text().trimmed();
    config.port = static_cast<quint16>(portEdit_->text().trimmed().toUShort());
    if (currentConnectionTabIndex_ == 0) {
        localConnectionConfig_ = config;
    } else {
        remoteConnectionConfig_ = config;
    }
}

void SettingsPage::refreshConnectionUiState() {
    QString text;
    QString style = "QLabel { color: #6b7280; }";
    if (!connectionMessage_.isEmpty() && !connected_) {
        text = connectionMessage_;
        style = connectionMessage_.contains("中")
            ? "QLabel { color: #8a6d1f; font-weight: 600; }"
            : "QLabel { color: #b42318; font-weight: 600; }";
    } else if (connected_) {
        text = QString("已连接  %1:%2").arg(ipEdit_->text().trimmed()).arg(portEdit_->text().trimmed());
        style = "QLabel { color: #027a48; font-weight: 600; }";
    } else {
        text = "未连接";
    }
    connectionStateLabel_->setText(text);
    connectionStateLabel_->setStyleSheet(style);
    disconnectButton_->setEnabled(connected_);
}

void SettingsPage::refreshStorageUiState(bool saved) {
    QString suffix;
    QString color = "#222222";
    if (saved) {
        suffix = "  已保存";
        color = "#027a48";
    } else if (storageDirty_) {
        suffix = "  未保存";
        color = "#8a6d1f";
    }
    storageTitleLabel_->setText(QString("存储%1").arg(suffix));
    storageTitleLabel_->setStyleSheet(QString("QLabel { font-weight: 600; color: %1; }").arg(color));
}

void SettingsPage::refreshPipeUiState() {
    pipeLogEdit_->viewport()->update();
}

void SettingsPage::updateResponsiveLayout() {
    const bool compact = width() < kCompactBreakpoint;
    contentLayout_->setDirection(compact ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);
    leftColumn_->setMaximumWidth(compact ? QWIDGETSIZE_MAX : 290);
    pipeGroup_->setMinimumHeight(compact ? 220 : 0);
}

}  // namespace deviceapp
