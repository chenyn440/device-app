#pragma once

#include <QBoxLayout>
#include <QWidget>

#include "core/types.h"

class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QPlainTextEdit;
class QFrame;

namespace deviceapp {

class SettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);

    void setConnectionConfig(const DeviceConnectionConfig &config);
    void setConnectionState(bool connected, const QString &message = QString());

    QString storageDirectory() const;
    QString filePrefix() const;
    bool autoSaveEnabled() const;

signals:
    void connectRequested(const DeviceConnectionConfig &config);
    void disconnectRequested();

private:
    void resizeEvent(QResizeEvent *event) override;

    void buildUi();
    QWidget *buildConnectionSection();
    QWidget *buildStorageSection();
    QWidget *buildPipeSection();
    QWidget *buildStatusBar();
    void loadLocalSettings();
    void saveLocalSettings() const;
    void appendPipeLog(const QString &text);
    void syncConnectionFormFromCurrentTab();
    void saveCurrentConnectionForm();
    void refreshConnectionUiState();
    void refreshStorageUiState(bool saved = false);
    void refreshPipeUiState();
    void updateResponsiveLayout();

    QBoxLayout *contentLayout_;
    QBoxLayout *leftLayout_;
    QWidget *leftColumn_;
    QWidget *connectionTabBar_;
    QFrame *connectionGroup_;
    QFrame *storageGroup_;
    QFrame *pipeGroup_;
    QPushButton *localConnectionTabButton_;
    QPushButton *remoteConnectionTabButton_;
    QLabel *connectionTitleLabel_;
    QLabel *connectionStateLabel_;
    QLineEdit *ipEdit_;
    QLineEdit *portEdit_;
    QPushButton *connectButton_;
    QPushButton *disconnectButton_;
    QLabel *storageTitleLabel_;
    QLineEdit *pathEdit_;
    QPushButton *browseButton_;
    QLineEdit *prefixEdit_;
    QPushButton *applyStorageButton_;
    QCheckBox *autoSaveCheck_;
    QLineEdit *pipeInputEdit_;
    QPushButton *sendPipeButton_;
    QCheckBox *showSendCheck_;
    QCheckBox *showReceiveCheck_;
    QPushButton *clearPipeButton_;
    QPlainTextEdit *pipeLogEdit_;
    QLabel *runStatusLabel_;
    DeviceConnectionConfig localConnectionConfig_;
    DeviceConnectionConfig remoteConnectionConfig_;
    int currentConnectionTabIndex_ = 0;
    bool connected_ = false;
    bool storageDirty_ = false;
    QString connectionMessage_;
};

}  // namespace deviceapp
