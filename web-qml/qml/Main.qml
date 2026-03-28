import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtCharts 2.15
import "theme" as ThemeLib
import "components"

ApplicationWindow {
    id: root
    visible: true
    width: 1366
    height: 768
    title: "四极质谱上位机"
    color: ThemeLib.Theme.color.appBg

    property int currentPageIndex: 0
    property int monitorSelectedRow: -1
    property int settingsConnectionTabIndex: 0
    property string settingsLocalHost: appState.connectionHost
    property int settingsLocalPort: appState.connectionPort
    property string settingsRemoteHost: "127.0.0.1"
    property int settingsRemotePort: 9001
    property bool settingsAutoSave: true
    property string settingsRunStatus: "运行状态    未连接"
    property int scanTabIndex: 0
    property bool scanFullMode: true
    property bool scanUseTargetVoltage: false
    property bool scanUseMassAxis: false
    property bool chartFixedY: false
    property bool controlForePump: false
    property bool controlForeValve: false
    property bool controlMolecularPump: false
    property bool controlInletValve: false
    property bool controlFilament: false
    property bool controlMultiplier: false
    property bool tuneForePumpOn: false
    property bool tuneForeValveOn: false
    property bool tuneMolecularPumpOn: false
    property bool tuneInletValveOn: false
    property bool tuneFilamentOn: false
    property bool tuneMultiplierOn: false
    property double controlSourceTemp: 0.0
    property double controlChamberTemp: 0.0
    property int runConnectPortValue: appState.connectionPort

    ListModel {
        id: simTableModel
        ListElement { enabled: true; mass: "18"; dwell: "80"; minv: "0"; maxv: "500" }
        ListElement { enabled: true; mass: "28"; dwell: "80"; minv: "0"; maxv: "500" }
        ListElement { enabled: true; mass: "44"; dwell: "80"; minv: "0"; maxv: "500" }
    }

    function f1(v) { return Number(v).toFixed(1) }
    function f2(v) { return Number(v).toFixed(2) }
    function requireConnected() {
        if (appState.connected) {
            return true
        }
        appState.toastRequested("设备未连接")
        return false
    }

    menuBar: MenuBar {
        implicitHeight: ThemeLib.Theme.font.menu + 18
        font.pixelSize: ThemeLib.Theme.font.menu
        Menu { title: "文件" }
        Menu { title: "运行" }
        Menu { title: "设置" }
        Menu { title: "视图" }
    }

    Connections {
        target: appState
        function onToastRequested(message) {
            toast.messageText = message
            toast.open()
        }
        function onStatusChanged() {
            if (appState.connected) {
                settingsRunStatus = "运行状态    已连接"
            } else if (settingsRunStatus !== "运行状态    存储设置已保存") {
                settingsRunStatus = "运行状态    未连接"
            }
            tuneForePumpOn = appState.switchState("forePump")
            tuneForeValveOn = appState.switchState("foreValve")
            tuneMolecularPumpOn = appState.switchState("molecularPump")
            tuneInletValveOn = appState.switchState("inletValve")
            tuneFilamentOn = appState.switchState("filament")
            tuneMultiplierOn = appState.switchState("multiplier")
        }
    }

    Dialog {
        id: toast
        modal: false
        standardButtons: Dialog.Ok
        property string messageText: ""
        contentItem: Label {
            text: toast.messageText
            wrapMode: Text.WordWrap
            padding: 10
        }
        x: root.width - width - 20
        y: 16
    }

    Menu {
        id: fileMenuPopup
        MenuItem { text: "打开单帧数据"; onTriggered: root.currentPageIndex = 3 }
        MenuItem { text: "退出"; onTriggered: Qt.quit() }
    }
    Menu {
        id: runMenuPopup
        MenuItem {
            text: "仪器连接"
            onTriggered: {
                runConnectHost.text = appState.connectionHost
                runConnectPortValue = appState.connectionPort
                runConnectionDialog.open()
            }
        }
        MenuItem { text: "断开连接"; onTriggered: appState.disconnectFromDevice() }
        MenuItem { text: "仪器控制"; onTriggered: instrumentControlDialog.open() }
    }
    Menu {
        id: settingMenuPopup
        MenuItem { text: "扫描设置"; onTriggered: scanSettingsDialog.open() }
        MenuItem { text: "数据处理"; onTriggered: dataProcessingDialog.open() }
        MenuItem { text: "系统设置"; onTriggered: systemSettingsDialog.open() }
        MenuItem { text: "谱图设置"; onTriggered: chartSettingsDialog.open() }
    }
    Menu {
        id: viewMenuPopup
        MenuItem { text: "切换到调谐"; onTriggered: root.currentPageIndex = 0 }
        MenuItem { text: "切换到监测"; onTriggered: root.currentPageIndex = 1 }
        MenuItem { text: "切换到方法"; onTriggered: root.currentPageIndex = 2 }
        MenuItem { text: "切换到数据"; onTriggered: root.currentPageIndex = 3 }
        MenuItem { text: "切换到设置"; onTriggered: root.currentPageIndex = 4 }
    }

    Dialog {
        id: runConnectionDialog
        title: "仪器连接"
        modal: true
        width: 408
        height: 256
        anchors.centerIn: parent
        standardButtons: Dialog.NoButton
        contentItem: Rectangle {
            color: "#ffffff"
            border.color: "#d0d5dd"
            radius: 2
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120
                    color: "#f9fafb"
                    border.color: "#d0d5dd"

                    GridLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        columns: 2
                        rowSpacing: 10
                        columnSpacing: 10

                        Label { text: "IP"; Layout.preferredWidth: 30 }
                        TextField {
                            id: runConnectHost
                            Layout.fillWidth: true
                            implicitHeight: 30
                            text: "127.0.0.1"
                            padding: 8
                        }

                        Label { text: "端口"; Layout.preferredWidth: 30 }
                        Rectangle {
                            Layout.preferredWidth: 150
                            Layout.preferredHeight: 30
                            color: "#ffffff"
                            border.color: "#cfd4dc"

                            RowLayout {
                                anchors.fill: parent
                                spacing: 0
                                Rectangle {
                                    Layout.preferredWidth: 38
                                    Layout.fillHeight: true
                                    color: "#f1f3f5"
                                    border.color: "#d1d5db"
                                    Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 20; color: "#475467" }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: runConnectPortValue = Math.max(1, runConnectPortValue - 1)
                                    }
                                }
                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    color: "#ffffff"
                                    Text {
                                        anchors.centerIn: parent
                                        text: Number(runConnectPortValue).toLocaleString(Qt.locale("en_US"), 'f', 0)
                                        font.pixelSize: 12
                                        color: "#344054"
                                    }
                                }
                                Rectangle {
                                    Layout.preferredWidth: 38
                                    Layout.fillHeight: true
                                    color: "#f1f3f5"
                                    border.color: "#d1d5db"
                                    Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 24; color: "#101828" }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: runConnectPortValue = Math.min(65535, runConnectPortValue + 1)
                                    }
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 52
                    color: "transparent"
                    border.color: "transparent"
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 14
                        anchors.rightMargin: 14
                        spacing: 12
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 32
                            radius: 2
                            border.color: "#d0d5dd"
                            color: "#f5f6f8"
                            Text { anchors.centerIn: parent; text: "取消"; color: "#334155"; font.pixelSize: 12 }
                            MouseArea { anchors.fill: parent; onClicked: runConnectionDialog.reject() }
                        }
                        Rectangle {
                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 32
                            radius: 2
                            border.color: "#1d64b7"
                            color: "#2a7de1"
                            Text { anchors.centerIn: parent; text: "确定"; color: "#ffffff"; font.pixelSize: 12 }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    runConnectionDialog.accept()
                                    appState.connectToDevice(runConnectHost.text, runConnectPortValue)
                                }
                            }
                        }
                        Item { Layout.fillWidth: true }
                    }
                }
            }
        }
    }

    Dialog {
        id: instrumentControlDialog
        title: "仪器控制"
        modal: true
        width: 430
        height: 500
        anchors.centerIn: parent
        standardButtons: Dialog.NoButton
        contentItem: Rectangle {
            color: "#ffffff"
            border.color: "#d0d5dd"
            radius: 2
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10

                GroupBox {
                    title: "开关控制"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 208
                    GridLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        columns: 3
                        rowSpacing: 6
                        columnSpacing: 10
                        CheckBox {
                            text: "前级泵"
                            checked: root.controlForePump
                            onToggled: root.controlForePump = checked
                            font.pixelSize: 12
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                            indicator: Rectangle { implicitWidth: 14; implicitHeight: 14; border.color: "#bfc6d0"; color: "white" }
                        }
                        CheckBox {
                            text: "前级阀"
                            checked: root.controlForeValve
                            onToggled: root.controlForeValve = checked
                            font.pixelSize: 12
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                            indicator: Rectangle { implicitWidth: 14; implicitHeight: 14; border.color: "#bfc6d0"; color: "white" }
                        }
                        CheckBox {
                            text: "分子泵"
                            checked: root.controlMolecularPump
                            onToggled: root.controlMolecularPump = checked
                            font.pixelSize: 12
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                            indicator: Rectangle { implicitWidth: 14; implicitHeight: 14; border.color: "#bfc6d0"; color: "white" }
                        }
                        CheckBox {
                            text: "进样阀"
                            checked: root.controlInletValve
                            onToggled: root.controlInletValve = checked
                            font.pixelSize: 12
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                            indicator: Rectangle { implicitWidth: 14; implicitHeight: 14; border.color: "#bfc6d0"; color: "white" }
                        }
                        CheckBox {
                            text: "灯丝"
                            checked: root.controlFilament
                            onToggled: root.controlFilament = checked
                            font.pixelSize: 12
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                            indicator: Rectangle { implicitWidth: 14; implicitHeight: 14; border.color: "#bfc6d0"; color: "white" }
                        }
                        CheckBox {
                            text: "倍增器"
                            checked: root.controlMultiplier
                            onToggled: root.controlMultiplier = checked
                            font.pixelSize: 12
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                            indicator: Rectangle { implicitWidth: 14; implicitHeight: 14; border.color: "#bfc6d0"; color: "white" }
                        }
                    }
                }

                GroupBox {
                    title: "温度控制"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 142
                    GridLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        columns: 2
                        rowSpacing: 8
                        columnSpacing: 10
                        Label { text: "离子源目标温度"; font.pixelSize: 12 }
                        Rectangle {
                            Layout.preferredWidth: 132
                            Layout.preferredHeight: 34
                            color: "#ffffff"
                            border.color: "#d0d5dd"
                            RowLayout {
                                anchors.fill: parent
                                spacing: 0
                                Rectangle {
                                    Layout.preferredWidth: 34
                                    Layout.fillHeight: true
                                    color: "#f2f4f7"
                                    Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 16; color: "#98a2b3" }
                                    MouseArea { anchors.fill: parent; onClicked: root.controlSourceTemp = Math.max(0, root.controlSourceTemp - 1) }
                                }
                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    color: "#ffffff"
                                    Text { anchors.centerIn: parent; text: Math.round(root.controlSourceTemp); font.pixelSize: 16; color: "#344054" }
                                }
                                Rectangle {
                                    Layout.preferredWidth: 34
                                    Layout.fillHeight: true
                                    color: "#f2f4f7"
                                    Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 22; color: "#101828" }
                                    MouseArea { anchors.fill: parent; onClicked: root.controlSourceTemp = Math.min(500, root.controlSourceTemp + 1) }
                                }
                            }
                        }

                        Label { text: "腔体母线温度"; font.pixelSize: 12 }
                        Rectangle {
                            Layout.preferredWidth: 132
                            Layout.preferredHeight: 34
                            color: "#ffffff"
                            border.color: "#d0d5dd"
                            RowLayout {
                                anchors.fill: parent
                                spacing: 0
                                Rectangle {
                                    Layout.preferredWidth: 34
                                    Layout.fillHeight: true
                                    color: "#f2f4f7"
                                    Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 16; color: "#98a2b3" }
                                    MouseArea { anchors.fill: parent; onClicked: root.controlChamberTemp = Math.max(0, root.controlChamberTemp - 1) }
                                }
                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    color: "#ffffff"
                                    Text { anchors.centerIn: parent; text: Math.round(root.controlChamberTemp); font.pixelSize: 16; color: "#344054" }
                                }
                                Rectangle {
                                    Layout.preferredWidth: 34
                                    Layout.fillHeight: true
                                    color: "#f2f4f7"
                                    Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 22; color: "#101828" }
                                    MouseArea { anchors.fill: parent; onClicked: root.controlChamberTemp = Math.min(200, root.controlChamberTemp + 1) }
                                }
                            }
                        }
                    }
                }

                Item { Layout.fillHeight: true }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 52
                    color: "#f3f4f6"
                    border.color: "#d0d5dd"
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 14
                        anchors.rightMargin: 14
                        spacing: 12
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 32
                            radius: 2
                            border.color: "#d0d5dd"
                            color: "#f5f6f8"
                            Text { anchors.centerIn: parent; text: "取消"; color: "#334155"; font.pixelSize: 12 }
                            MouseArea { anchors.fill: parent; onClicked: instrumentControlDialog.reject() }
                        }
                        Rectangle {
                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 32
                            radius: 2
                            border.color: "#1d64b7"
                            color: "#2a7de1"
                            Text { anchors.centerIn: parent; text: "确定"; color: "#ffffff"; font.pixelSize: 12 }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    instrumentControlDialog.accept()
                                    appState.toastRequested("仪器控制设置已应用")
                                }
                            }
                        }
                        Item { Layout.fillWidth: true }
                    }
                }
            }
        }
    }

    Dialog {
        id: scanSettingsDialog
        title: "扫描设置"
        modal: true
        width: 840
        height: 680
        anchors.centerIn: parent
        standardButtons: Dialog.NoButton
        onAccepted: appState.toastRequested("扫描设置已应用")
        onOpened: root.scanTabIndex = 0
        contentItem: Rectangle {
            color: "#ffffff"
            border.color: "#d0d5dd"
            radius: 2

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 12

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 54
                    color: "#f3f4f6"
                    border.color: "#cfd4dc"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 0

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: root.scanTabIndex === 0 ? "#ffffff" : "#3f444a"
                            border.color: "#c8ced7"
                            Text {
                                anchors.centerIn: parent
                                text: "扫描参数"
                                font.pixelSize: 23 / 2
                                font.bold: true
                                color: root.scanTabIndex === 0 ? "#1f2937" : "#ffffff"
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: root.scanTabIndex = 0
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: root.scanTabIndex === 1 ? "#ffffff" : "#3f444a"
                            border.color: "#c8ced7"
                            Text {
                                anchors.centerIn: parent
                                text: "质量轴"
                                font.pixelSize: 23 / 2
                                font.bold: true
                                color: root.scanTabIndex === 1 ? "#1f2937" : "#ffffff"
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: root.scanTabIndex = 1
                            }
                        }
                    }
                }

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: root.scanTabIndex

                    ScrollView {
                        clip: true
                        ColumnLayout {
                            width: parent.width
                            spacing: 12

                            GroupBox {
                                title: ""
                                Layout.fillWidth: true
                                background: Rectangle { color: "transparent"; border.color: "transparent"; radius: 0 }

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 0
                                    spacing: 12

                                    GroupBox {
                                        title: "扫描方式"
                                        Layout.fillWidth: true
                                        background: Rectangle { color: "#ffffff"; border.color: "#d8dbe2"; radius: 2 }
                                        ColumnLayout {
                                            anchors.fill: parent
                                            anchors.margins: 10
                                            spacing: 10
                                            RowLayout {
                                                RadioButton {
                                                    text: "全扫描"
                                                    checked: root.scanFullMode
                                                    onToggled: if (checked) root.scanFullMode = true
                                                    font.pixelSize: 12
                                                    indicator: Rectangle {
                                                        implicitWidth: 14
                                                        implicitHeight: 14
                                                        radius: 7
                                                        border.color: "#b8c0cc"
                                                        color: "#f8fafc"
                                                        Rectangle {
                                                            anchors.centerIn: parent
                                                            width: 8
                                                            height: 8
                                                            radius: 4
                                                            color: "#3f444a"
                                                            visible: parent.parent.checked
                                                        }
                                                    }
                                                }
                                                RadioButton {
                                                    text: "选择离子扫描"
                                                    checked: !root.scanFullMode
                                                    onToggled: if (checked) root.scanFullMode = false
                                                    font.pixelSize: 12
                                                    indicator: Rectangle {
                                                        implicitWidth: 14
                                                        implicitHeight: 14
                                                        radius: 7
                                                        border.color: "#b8c0cc"
                                                        color: "#f8fafc"
                                                        Rectangle {
                                                            anchors.centerIn: parent
                                                            width: 8
                                                            height: 8
                                                            radius: 4
                                                            color: "#3f444a"
                                                            visible: parent.parent.checked
                                                        }
                                                    }
                                                }
                                                Item { Layout.fillWidth: true }
                                            }
                                            GroupBox {
                                                title: "质量轴设置"
                                                Layout.fillWidth: true
                                                background: Rectangle { color: "transparent"; border.color: "transparent"; radius: 0 }
                                                ColumnLayout {
                                                    anchors.fill: parent
                                                    spacing: 8

                                                    RowLayout {
                                                        Layout.fillWidth: true
                                                        spacing: 8
                                                        Label { text: "K"; Layout.preferredWidth: 88 }
                                                        TextField { text: "0.000000"; implicitHeight: 28; Layout.preferredWidth: 200 }
                                                        Label { text: "B"; Layout.preferredWidth: 16 }
                                                        TextField { text: "0.0000"; implicitHeight: 28; Layout.preferredWidth: 200 }
                                                        Item { Layout.fillWidth: true }
                                                    }

                                                    RowLayout {
                                                        Layout.fillWidth: true
                                                        spacing: 8
                                                        Label { text: "是否使用质量轴"; Layout.preferredWidth: 88 }
                                                        Label { text: root.scanUseMassAxis ? "是" : "否"; Layout.preferredWidth: 20 }
                                                        AppButton { text: "切换到质量轴页"; implicitWidth: 116; onClicked: root.scanTabIndex = 1 }
                                                        Item { Layout.fillWidth: true }
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    GroupBox {
                                        title: "目标离子"
                                        Layout.fillWidth: true
                                        GridLayout {
                                            columns: 2
                                            rowSpacing: 8
                                            columnSpacing: 8
                                            Label { text: "质量数"; Layout.alignment: Qt.AlignTop }
                                            RowLayout {
                                                spacing: 10
                                                AppCheckBox { text: "61"; checked: true }
                                                AppCheckBox { text: "131"; checked: true }
                                                AppCheckBox { text: "219" }
                                                AppCheckBox { text: "264" }
                                                AppCheckBox { text: "414" }
                                                AppCheckBox { text: "502" }
                                                Item { Layout.fillWidth: true }
                                            }
                                            Label { text: "电压设置"; Layout.alignment: Qt.AlignTop }
                                            RowLayout {
                                                spacing: 6
                                                Repeater {
                                                    model: 6
                                                    delegate: TextField {
                                                        implicitWidth: 86
                                                        implicitHeight: 28
                                                        text: "0.0"
                                                        enabled: root.scanUseTargetVoltage
                                                    }
                                                }
                                                Item { Layout.fillWidth: true }
                                            }
                                        }
                                    }

                                    GroupBox {
                                        title: "全扫描设置"
                                        Layout.fillWidth: true
                                        visible: root.scanFullMode
                                        GridLayout {
                                            columns: 4
                                            rowSpacing: 8
                                            columnSpacing: 8
                                            Label { text: "质量数"; Layout.preferredWidth: 72 }
                                            RowLayout {
                                                Layout.columnSpan: 3
                                                spacing: 6
                                                TextField { id: fullScanMassStart; implicitWidth: 90; implicitHeight: 28; text: "10.0" }
                                                Label { text: "至" }
                                                TextField { id: fullScanMassEnd; implicitWidth: 90; implicitHeight: 28; text: "110.0" }
                                                Item { Layout.fillWidth: true }
                                            }
                                            Label { text: "扫描速度"; Layout.preferredWidth: 72 }
                                            TextField { id: scanSpeedField; implicitHeight: 28; text: "1000.0" }
                                            Label { text: "扫描时间"; Layout.preferredWidth: 72 }
                                            RowLayout {
                                                ComboBox {
                                                    id: scanTimePresetBox
                                                    model: ["100 ms", "200 ms", "500 ms", "1000 ms", "自定义"]
                                                    currentIndex: 2
                                                    implicitHeight: 28
                                                    onActivated: {
                                                        if (currentText === "100 ms") customScanTimeField.text = "100.0"
                                                        else if (currentText === "200 ms") customScanTimeField.text = "200.0"
                                                        else if (currentText === "500 ms") customScanTimeField.text = "500.0"
                                                        else if (currentText === "1000 ms") customScanTimeField.text = "1000.0"
                                                    }
                                                }
                                                TextField {
                                                    id: customScanTimeField
                                                    implicitWidth: 96
                                                    implicitHeight: 28
                                                    text: "500.0"
                                                    enabled: scanTimePresetBox.currentText === "自定义"
                                                }
                                            }
                                            Label { text: "回扫时间"; Layout.preferredWidth: 72 }
                                            TextField { id: flybackTimeField; implicitHeight: 28; text: "100.0" }
                                            AppCheckBox {
                                                id: useTargetVoltageCheck
                                                text: "是否使用电压"
                                                checked: root.scanUseTargetVoltage
                                                onToggled: root.scanUseTargetVoltage = checked
                                            }
                                            RowLayout {
                                                spacing: 6
                                                TextField { id: voltageMinField; implicitWidth: 80; implicitHeight: 28; text: "0.0"; enabled: root.scanUseTargetVoltage }
                                                Label { text: "至" }
                                                TextField { id: voltageMaxField; implicitWidth: 80; implicitHeight: 28; text: "0.0"; enabled: root.scanUseTargetVoltage }
                                                AppButton {
                                                    text: "计算"
                                                    implicitWidth: 58
                                                    enabled: root.scanUseTargetVoltage
                                                    onClicked: {
                                                        const minV = Number(voltageMinField.text)
                                                        const maxV = Number(voltageMaxField.text)
                                                        if (isNaN(minV) || isNaN(maxV)) {
                                                            appState.toastRequested("请输入有效电压范围")
                                                            return
                                                        }
                                                        appState.toastRequested("已按电压范围计算: " + minV.toFixed(1) + " ~ " + maxV.toFixed(1))
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    GroupBox {
                                        title: "选择离子扫描设置"
                                        Layout.fillWidth: true
                                        visible: !root.scanFullMode
                                        GridLayout {
                                            columns: 4
                                            rowSpacing: 8
                                            columnSpacing: 8
                                            Label { text: "驻留时间"; Layout.preferredWidth: 72 }
                                            TextField { id: selectedDwellTimeField; implicitHeight: 28; text: "5.0" }
                                            Label { text: "回扫时间"; Layout.preferredWidth: 72 }
                                            TextField { id: selectedFlybackField; implicitHeight: 28; text: "100.0" }
                                            Label { text: "目标峰宽度"; Layout.preferredWidth: 72 }
                                            TextField { id: selectedPeakWidthField; implicitHeight: 28; text: "1.0" }
                                            Label { text: "斜扫电压"; Layout.preferredWidth: 72 }
                                            TextField { id: selectedRampVoltageField; implicitHeight: 28; text: "120.0" }
                                        }
                                    }

                                    GroupBox {
                                        title: "RFAD 设置"
                                        Layout.fillWidth: true
                                        RowLayout {
                                            anchors.fill: parent
                                            anchors.margins: 10
                                            spacing: 8
                                            Label { text: "采样频率" }
                                            ComboBox { id: rfadSampleRate; model: ["10K", "20K", "50K", "100K"]; currentIndex: 1; implicitHeight: 28 }
                                            Item { Layout.preferredWidth: 16 }
                                            AppButton { text: "设置"; implicitWidth: 58; onClicked: appState.toastRequested("RFAD 设置已更新") }
                                            Item { Layout.fillWidth: true }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    ScrollView {
                        clip: true
                        ColumnLayout {
                            width: parent.width
                            spacing: 12
                            GroupBox {
                                title: "质量轴"
                                Layout.fillWidth: true
                                background: Rectangle { color: "#ffffff"; border.color: "#d0d5dd"; radius: 2 }
                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 10
                                    AppCheckBox {
                                        id: useMassAxisCheck
                                        text: "启用质量轴校准"
                                        checked: root.scanUseMassAxis
                                        onToggled: root.scanUseMassAxis = checked
                                    }
                                    GridLayout {
                                        columns: 2
                                        rowSpacing: 8
                                        columnSpacing: 10
                                        Label { text: "质量轴 K" }
                                        TextField { implicitHeight: 28; text: "0.000000" }
                                        Label { text: "质量轴 B" }
                                        TextField { implicitHeight: 28; text: "0.0000" }
                                        Label { text: "采样点数" }
                                        TextField { implicitHeight: 28; text: "1024" }
                                    }
                                    AppButton { text: "全部应用"; implicitWidth: 74 }
                                    Rectangle {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 44
                                        color: "#f8fafc"
                                        border.color: "#d0d5dd"
                                        Label {
                                            anchors.fill: parent
                                            anchors.margins: 8
                                            text: "用于恢复质量轴修正参数；未启用时按默认轴计算。"
                                            wrapMode: Text.WordWrap
                                            color: "#475467"
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 54
                    color: "#f3f4f6"
                    border.color: "#d0d5dd"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 14
                        anchors.rightMargin: 14
                        spacing: 12
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 32
                            radius: 2
                            color: "#f5f6f8"
                            border.color: "#b9c1cb"
                            Text {
                                anchors.centerIn: parent
                                text: "取消"
                                font.pixelSize: 12
                                color: "#334155"
                            }
                            MouseArea { anchors.fill: parent; onClicked: scanSettingsDialog.reject() }
                        }
                        Rectangle {
                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 32
                            radius: 2
                            color: "#2a7de1"
                            border.color: "#1d64b7"
                            Text {
                                anchors.centerIn: parent
                                text: "确定"
                                font.pixelSize: 12
                                color: "#ffffff"
                            }
                            MouseArea { anchors.fill: parent; onClicked: scanSettingsDialog.accept() }
                        }
                        Item { Layout.fillWidth: true }
                    }
                }
            }
        }
    }

    Dialog {
        id: dataProcessingDialog
        title: "数据处理"
        modal: true
        width: 420
        anchors.centerIn: parent
        standardButtons: Dialog.NoButton
        contentItem: Rectangle {
            color: "#ffffff"
            border.color: "#d0d5dd"
            radius: 2
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10
                AppCheckBox { text: "启用平滑滤波" }
                RowLayout {
                    Label { text: "平滑窗口" }
                    SpinBox { from: 1; to: 99; value: 11 }
                }
                RowLayout {
                    Label { text: "基线 Lambda" }
                    TextField { text: "1000.0" }
                }
                RowLayout {
                    Label { text: "基线 Asymmetry" }
                    TextField { text: "0.0010" }
                }
                Item { Layout.fillHeight: true }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 52
                    color: "#f3f4f6"
                    border.color: "#d0d5dd"
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 14
                        anchors.rightMargin: 14
                        spacing: 12
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 32
                            radius: 2
                            border.color: "#d0d5dd"
                            color: "#f5f6f8"
                            Text { anchors.centerIn: parent; text: "取消"; color: "#334155"; font.pixelSize: 12 }
                            MouseArea { anchors.fill: parent; onClicked: dataProcessingDialog.reject() }
                        }
                        Rectangle {
                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 32
                            radius: 2
                            border.color: "#1d64b7"
                            color: "#2a7de1"
                            Text { anchors.centerIn: parent; text: "确定"; color: "#ffffff"; font.pixelSize: 12 }
                            MouseArea { anchors.fill: parent; onClicked: dataProcessingDialog.accept() }
                        }
                        Item { Layout.fillWidth: true }
                    }
                }
            }
        }
    }

    Dialog {
        id: systemSettingsDialog
        title: "系统设置"
        modal: true
        width: 520
        height: 420
        anchors.centerIn: parent
        standardButtons: Dialog.NoButton
        contentItem: Rectangle {
            color: "#ffffff"
            border.color: "#d0d5dd"
            radius: 2
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10
                GroupBox {
                    title: "RFDA 设置"
                    Layout.fillWidth: true
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        Label { text: "RFDDS" }
                        TextField { Layout.fillWidth: true }
                        AppButton { text: "设置"; implicitWidth: 56 }
                    }
                }
                GroupBox {
                    title: "系统信息"
                    Layout.fillWidth: true
                    GridLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        columns: 2
                        Label { text: "编号" }
                        TextField { Layout.fillWidth: true }
                        Label { text: "型号" }
                        ComboBox { model: ["QMS-100", "QMS-200", "QMS-300"]; Layout.fillWidth: true }
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    Item { Layout.fillWidth: true }
                    AppButton { text: "设置"; implicitWidth: 56 }
                }
                Item { Layout.fillHeight: true }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 52
                    color: "#f3f4f6"
                    border.color: "#d0d5dd"
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 14
                        anchors.rightMargin: 14
                        spacing: 12
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 32
                            radius: 2
                            border.color: "#d0d5dd"
                            color: "#f5f6f8"
                            Text { anchors.centerIn: parent; text: "取消"; color: "#334155"; font.pixelSize: 12 }
                            MouseArea { anchors.fill: parent; onClicked: systemSettingsDialog.reject() }
                        }
                        Rectangle {
                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 32
                            radius: 2
                            border.color: "#1d64b7"
                            color: "#2a7de1"
                            Text { anchors.centerIn: parent; text: "确定"; color: "#ffffff"; font.pixelSize: 12 }
                            MouseArea { anchors.fill: parent; onClicked: systemSettingsDialog.accept() }
                        }
                        Item { Layout.fillWidth: true }
                    }
                }
            }
        }
    }

    Dialog {
        id: chartSettingsDialog
        title: "谱图设置"
        modal: true
        width: 900
        height: 420
        anchors.centerIn: parent
        standardButtons: Dialog.NoButton
        onAccepted: appState.toastRequested("谱图设置已应用")
        contentItem: Rectangle {
            color: "#ffffff"
            border.color: "#d0d5dd"
            radius: 2

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#ffffff"
                    border.color: "#d0d5dd"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        ColumnLayout {
                            Layout.preferredWidth: 170
                            Layout.fillHeight: true
                            spacing: 14
                            RadioButton {
                                text: "质量数"
                                font.pixelSize: 12
                                indicator: Rectangle {
                                    implicitWidth: 14
                                    implicitHeight: 14
                                    radius: 7
                                    border.color: "#b8c0cc"
                                    color: "#f8fafc"
                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: 8
                                        height: 8
                                        radius: 4
                                        color: "#3f444a"
                                        visible: parent.parent.checked
                                    }
                                }
                            }
                            RadioButton {
                                text: "电压"
                                font.pixelSize: 12
                                indicator: Rectangle {
                                    implicitWidth: 14
                                    implicitHeight: 14
                                    radius: 7
                                    border.color: "#b8c0cc"
                                    color: "#f8fafc"
                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: 8
                                        height: 8
                                        radius: 4
                                        color: "#3f444a"
                                        visible: parent.parent.checked
                                    }
                                }
                            }
                            RadioButton {
                                text: "时间"
                                font.pixelSize: 12
                                indicator: Rectangle {
                                    implicitWidth: 14
                                    implicitHeight: 14
                                    radius: 7
                                    border.color: "#b8c0cc"
                                    color: "#f8fafc"
                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: 8
                                        height: 8
                                        radius: 4
                                        color: "#3f444a"
                                        visible: parent.parent.checked
                                    }
                                }
                            }
                            RadioButton {
                                text: "点数"
                                checked: true
                                font.pixelSize: 12
                                indicator: Rectangle {
                                    implicitWidth: 14
                                    implicitHeight: 14
                                    radius: 7
                                    border.color: "#b8c0cc"
                                    color: "#f8fafc"
                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: 8
                                        height: 8
                                        radius: 4
                                        color: "#3f444a"
                                        visible: parent.parent.checked
                                    }
                                }
                            }
                            Item { Layout.fillHeight: true }
                        }

                        Rectangle {
                            Layout.preferredWidth: 1
                            Layout.fillHeight: true
                            color: "#d0d5dd"
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: 12
                            AppCheckBox { text: "谱图不自动恢复全屏" }
                            AppCheckBox { text: "Y轴范围固定"; checked: root.chartFixedY; onToggled: root.chartFixedY = checked }
                            Item { Layout.preferredHeight: 8 }
                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: "最小值"; Layout.preferredWidth: 56 }
                                TextField { Layout.fillWidth: true; implicitHeight: 40; enabled: root.chartFixedY; text: "0.0" }
                            }
                            Item { Layout.preferredHeight: 10 }
                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: "最大值"; Layout.preferredWidth: 56 }
                                TextField { Layout.fillWidth: true; implicitHeight: 40; enabled: root.chartFixedY; text: "1000.0" }
                            }
                            Item { Layout.fillHeight: true }
                        }

                        Rectangle {
                            Layout.preferredWidth: 1
                            Layout.fillHeight: true
                            color: "#d0d5dd"
                        }

                        ColumnLayout {
                            Layout.preferredWidth: 260
                            Layout.fillHeight: true
                            spacing: 12
                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: "谱峰个数"; Layout.preferredWidth: 70 }
                                SpinBox { from: 0; to: 999; value: 8; editable: true; implicitHeight: 40; implicitWidth: 140 }
                            }
                            AppCheckBox { text: "显示原始数据"; checked: true }
                            AppCheckBox { text: "显示余晖" }
                            AppCheckBox { text: "显示半峰宽" }
                            Item { Layout.fillHeight: true }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 54
                    color: "#f3f4f6"
                    border.color: "#d0d5dd"
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 14
                        anchors.rightMargin: 14
                        spacing: 12
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 32
                            radius: 2
                            color: "#f5f6f8"
                            border.color: "#d0d5dd"
                            Text { anchors.centerIn: parent; text: "取消"; font.pixelSize: 12; color: "#334155" }
                            MouseArea { anchors.fill: parent; onClicked: chartSettingsDialog.reject() }
                        }
                        Rectangle {
                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 32
                            radius: 2
                            color: "#2a7de1"
                            border.color: "#1d64b7"
                            Text { anchors.centerIn: parent; text: "确定"; font.pixelSize: 12; color: "#ffffff" }
                            MouseArea { anchors.fill: parent; onClicked: chartSettingsDialog.accept() }
                        }
                        Item { Layout.fillWidth: true }
                    }
                }
            }
        }
    }

    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 34
        color: "#f5f7fa"
        border.color: "#d0d5dd"

        RowLayout {
            id: topMenuRow
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            spacing: 2

            Repeater {
                model: [
                    { label: "文件", menu: fileMenuPopup },
                    { label: "运行", menu: runMenuPopup },
                    { label: "设置", menu: settingMenuPopup },
                    { label: "视图", menu: viewMenuPopup }
                ]
                delegate: Rectangle {
                    required property var modelData
                    Layout.preferredWidth: 58
                    Layout.preferredHeight: 24
                    radius: 2
                    color: menuMouse.containsMouse ? "#e7edf5" : "transparent"
                    Text {
                        anchors.centerIn: parent
                        text: modelData.label
                        font.pixelSize: 13
                        color: "#101828"
                    }
                    MouseArea {
                        id: menuMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            const p = parent.mapToItem(root.contentItem, 0, parent.height + 2)
                            modelData.menu.x = p.x
                            modelData.menu.y = p.y
                            modelData.menu.open()
                        }
                    }
                }
            }
            Item { Layout.fillWidth: true }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.topMargin: 34
        spacing: 0

        Rectangle {
            Layout.preferredWidth: ThemeLib.Theme.size.navW
            Layout.fillHeight: true
            color: "#dfe4ea"
            border.color: "#c9d0da"

            ColumnLayout {
                anchors.fill: parent
                spacing: 0
                Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 38; color: "transparent" }

                Repeater {
                    model: ["调谐", "监测", "方法", "数据", "设置"]
                    delegate: Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 62
                        color: index === root.currentPageIndex ? "#d7e6ff" : "transparent"
                        border.width: index === root.currentPageIndex ? 1 : 0
                        border.color: "#98b8f2"

                        Text {
                            anchors.centerIn: parent
                            text: modelData
                            font.pixelSize: 14
                            color: index === root.currentPageIndex ? "#2d7ef7" : "#4b5563"
                            font.bold: index === root.currentPageIndex
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: root.currentPageIndex = index
                            cursorShape: Qt.PointingHandCursor
                        }
                    }
                }
                Item { Layout.fillHeight: true }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: ThemeLib.Theme.color.workspaceBg

            StackLayout {
                anchors.fill: parent
                anchors.margins: 8
                currentIndex: root.currentPageIndex

                Item {
                    id: tunePage
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 6

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 31
                            color: ThemeLib.Theme.color.toolbarBg
                            border.color: ThemeLib.Theme.color.borderStrong

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 3
                                spacing: 4

                                AppButton {
                                    text: "开始"
                                    selected: appState.scanning
                                    enabled: appState.connected && !appState.scanning
                                    onClicked: appState.startScan(tuneMassStart.value, tuneMassEnd.value)
                                }
                                AppButton {
                                    text: "停止"
                                    selected: !appState.scanning
                                    enabled: appState.connected && appState.scanning
                                    onClicked: appState.stopScan()
                                }
                                AppButton {
                                    text: "校正当前质量轴"
                                    implicitWidth: 102
                                    onClicked: {
                                        if (!root.requireConnected()) return
                                        appState.calibrateMassAxis()
                                    }
                                }
                                AppButton {
                                    text: tuneForePumpOn ? "前级泵✓" : "前级泵"
                                    selected: tuneForePumpOn
                                    onClicked: {
                                        tuneForePumpOn = !tuneForePumpOn
                                        appState.setInstrumentSwitch("forePump", tuneForePumpOn)
                                    }
                                }
                                AppButton {
                                    text: tuneForeValveOn ? "前级阀✓" : "前级阀"
                                    selected: tuneForeValveOn
                                    onClicked: {
                                        tuneForeValveOn = !tuneForeValveOn
                                        appState.setInstrumentSwitch("foreValve", tuneForeValveOn)
                                    }
                                }
                                AppButton {
                                    text: tuneMolecularPumpOn ? "分子泵✓" : "分子泵"
                                    selected: tuneMolecularPumpOn
                                    onClicked: {
                                        tuneMolecularPumpOn = !tuneMolecularPumpOn
                                        appState.setInstrumentSwitch("molecularPump", tuneMolecularPumpOn)
                                    }
                                }
                                AppButton {
                                    text: tuneInletValveOn ? "进样阀✓" : "进样阀"
                                    selected: tuneInletValveOn
                                    onClicked: {
                                        tuneInletValveOn = !tuneInletValveOn
                                        appState.setInstrumentSwitch("inletValve", tuneInletValveOn)
                                    }
                                }
                                AppButton {
                                    text: tuneFilamentOn ? "灯丝✓" : "灯丝"
                                    selected: tuneFilamentOn
                                    onClicked: {
                                        tuneFilamentOn = !tuneFilamentOn
                                        appState.setInstrumentSwitch("filament", tuneFilamentOn)
                                    }
                                }
                                AppButton {
                                    text: tuneMultiplierOn ? "倍增器✓" : "倍增器"
                                    selected: tuneMultiplierOn
                                    onClicked: {
                                        tuneMultiplierOn = !tuneMultiplierOn
                                        appState.setInstrumentSwitch("multiplier", tuneMultiplierOn)
                                    }
                                }
                                AppButton {
                                    text: "灯丝切换"
                                    implicitWidth: 70
                                    onClicked: {
                                        appState.toggleDetectorMode()
                                    }
                                }
                                AppButton {
                                    text: "数据保存"
                                    implicitWidth: 66
                                    onClicked: {
                                        if (!root.requireConnected()) return
                                        appState.saveCurrentFrame()
                                    }
                                }
                                AppButton {
                                    text: "谱图刷新"
                                    implicitWidth: 66
                                    onClicked: appState.refreshCharts()
                                }
                                Item { Layout.fillWidth: true }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: 8

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                color: "#f4f6f8"
                                border.color: ThemeLib.Theme.color.borderDefault

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 4
                                    spacing: 6

                                    Rectangle {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 146
                                        color: "#eff2f6"
                                        border.color: ThemeLib.Theme.color.borderDefault

                                        RowLayout {
                                            anchors.fill: parent
                                            anchors.margins: 4
                                            spacing: 4

                                            Rectangle {
                                                Layout.preferredWidth: 236
                                                Layout.fillHeight: true
                                                color: "#ffffff"
                                                border.color: ThemeLib.Theme.color.borderSubtle

                                                ColumnLayout {
                                                    anchors.fill: parent
                                                    anchors.margins: 8
                                                    spacing: 6

                                                    RowLayout {
                                                        AppCheckBox { text: "Full Scan"; checked: true }
                                                        AppCheckBox { text: "SIM" }
                                                        Item { Layout.fillWidth: true }
                                                        AppButton { text: "设置"; implicitWidth: 46 }
                                                    }

                                                    GridLayout {
                                                        columns: 2
                                                        rowSpacing: 4
                                                        columnSpacing: 8
                                                        Label { text: "起始质量数："; font.pixelSize: 12 }
                                                        SpinBox {
                                                            id: tuneMassStart
                                                            from: 1
                                                            to: 500
                                                            value: 10
                                                            editable: true
                                                            implicitWidth: 120
                                                            implicitHeight: 24
                                                            leftPadding: 20
                                                            rightPadding: 20
                                                            down.indicator: Rectangle {
                                                                x: 0
                                                                y: 0
                                                                width: 18
                                                                height: tuneMassStart.height
                                                                color: "#eceff3"
                                                                border.color: "#cfd4dc"
                                                                Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 12; color: "#344054" }
                                                            }
                                                            up.indicator: Rectangle {
                                                                x: tuneMassStart.width - width
                                                                y: 0
                                                                width: 18
                                                                height: tuneMassStart.height
                                                                color: "#eceff3"
                                                                border.color: "#cfd4dc"
                                                                Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 12; color: "#344054" }
                                                            }
                                                        }
                                                        Label { text: "终止质量数："; font.pixelSize: 12 }
                                                        SpinBox {
                                                            id: tuneMassEnd
                                                            from: 1
                                                            to: 500
                                                            value: 110
                                                            editable: true
                                                            implicitWidth: 120
                                                            implicitHeight: 24
                                                            leftPadding: 20
                                                            rightPadding: 20
                                                            down.indicator: Rectangle {
                                                                x: 0
                                                                y: 0
                                                                width: 18
                                                                height: tuneMassEnd.height
                                                                color: "#eceff3"
                                                                border.color: "#cfd4dc"
                                                                Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 12; color: "#344054" }
                                                            }
                                                            up.indicator: Rectangle {
                                                                x: tuneMassEnd.width - width
                                                                y: 0
                                                                width: 18
                                                                height: tuneMassEnd.height
                                                                color: "#eceff3"
                                                                border.color: "#cfd4dc"
                                                                Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 12; color: "#344054" }
                                                            }
                                                        }
                                                    }
                                                }
                                            }

                                            Rectangle {
                                                Layout.preferredWidth: 236
                                                Layout.fillHeight: true
                                                color: "#ffffff"
                                                border.color: ThemeLib.Theme.color.borderSubtle

                                                ColumnLayout {
                                                    anchors.fill: parent
                                                    anchors.margins: 8
                                                    spacing: 6
                                                    Label { text: "检测器"; font.pixelSize: 12; font.bold: true }
                                                    RowLayout {
                                                        AppCheckBox { text: "倍增器"; checked: true }
                                                        AppCheckBox { text: "法拉第筒" }
                                                    }
                                                    Label { text: "稳定区"; font.pixelSize: 12; font.bold: true }
                                                    RowLayout {
                                                        AppCheckBox { text: "稳定区一"; checked: true }
                                                        AppCheckBox { text: "稳定区二" }
                                                    }
                                                }
                                            }

                                            Rectangle {
                                                Layout.fillWidth: true
                                                Layout.fillHeight: true
                                                color: "#ffffff"
                                                border.color: ThemeLib.Theme.color.borderSubtle

                                                ColumnLayout {
                                                    id: tuneSummaryTable
                                                    anchors.fill: parent
                                                    anchors.margins: 4
                                                    spacing: 0

                                                    property var rows: [
                                                        ["m/z", "18", "28", "44"],
                                                        ["强度[v]", f1(appState.peak18), f1(appState.peak28), f1(appState.peak44)],
                                                        ["丰度比[%]", f1(appState.peak18 / 20.0), f1(appState.peak28 / 20.0), f1(appState.peak44 / 20.0)],
                                                        ["半峰宽[m.z]", f2(1.20), f2(1.40), f2(1.60)]
                                                    ]

                                                    Repeater {
                                                        model: tuneSummaryTable.rows.length
                                                        delegate: RowLayout {
                                                            property int rowIndex: index
                                                            Layout.fillWidth: true
                                                            Layout.preferredHeight: 28
                                                            spacing: 0

                                                            Repeater {
                                                                model: 4
                                                                delegate: Rectangle {
                                                                    Layout.fillWidth: true
                                                                    Layout.preferredWidth: index === 0 ? 1.3 : 1.0
                                                                    Layout.fillHeight: true
                                                                    border.color: "#d8dde5"
                                                                    color: rowIndex === 0 ? "#f6f8fb" : "#ffffff"

                                                                    Text {
                                                                        anchors.centerIn: parent
                                                                        text: tuneSummaryTable.rows[rowIndex][index]
                                                                        font.pixelSize: 12
                                                                        font.bold: rowIndex === 0
                                                                        color: ThemeLib.Theme.color.textPrimary
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    Rectangle {
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true
                                        color: "#ffffff"
                                        border.color: ThemeLib.Theme.color.borderDefault

                                        ColumnLayout {
                                            anchors.fill: parent
                                            anchors.margins: 3
                                            spacing: 3

                                            ChartView {
                                                Layout.fillWidth: true
                                                Layout.fillHeight: true
                                                Layout.preferredHeight: 4.35
                                                antialiasing: true
                                                legend.visible: false
                                                title: "全扫描"
                                                backgroundColor: ThemeLib.Theme.color.chartBg
                                                plotAreaColor: ThemeLib.Theme.color.chartBg
                                                ValueAxis { id: sx; min: 10; max: 110 }
                                                ValueAxis { id: sy; min: 0; max: 1000 }
                                                LineSeries {
                                                    id: spectrumSeries
                                                    axisX: sx
                                                    axisY: sy
                                                    color: ThemeLib.Theme.color.linePrimary
                                                    width: 2
                                                }
                                                Component.onCompleted: appState.bindSpectrumSeries(spectrumSeries)
                                            }

                                            Rectangle {
                                                Layout.fillWidth: true
                                                height: 1
                                                color: "#cfd5de"
                                            }

                                            ChartView {
                                                Layout.fillWidth: true
                                                Layout.fillHeight: true
                                                Layout.preferredHeight: 1.55
                                                antialiasing: true
                                                legend.visible: false
                                                title: "TIC"
                                                backgroundColor: ThemeLib.Theme.color.chartBg
                                                plotAreaColor: ThemeLib.Theme.color.chartBg
                                                ValueAxis { id: tuneTx; min: 0; max: 240 }
                                                ValueAxis { id: tuneTy; min: 0; max: 250000 }
                                                LineSeries {
                                                    id: tuneTicSeries
                                                    axisX: tuneTx
                                                    axisY: tuneTy
                                                    color: ThemeLib.Theme.color.linePrimary
                                                    width: 2
                                                }
                                                Component.onCompleted: appState.bindTicSeries(tuneTicSeries)
                                            }
                                        }

                                        Rectangle {
                                            anchors.left: parent.left
                                            anchors.leftMargin: 4
                                            anchors.bottom: parent.bottom
                                            anchors.bottomMargin: 154
                                            width: 44
                                            height: 20
                                            color: "transparent"
                                            RowLayout {
                                                anchors.fill: parent
                                                spacing: 2
                                                Rectangle {
                                                    width: 18
                                                    height: 18
                                                    color: "#f0f2f5"
                                                    border.color: "#bfc6d0"
                                                    Text { anchors.centerIn: parent; text: "◀"; font.pixelSize: 10; color: "#3f4753" }
                                                }
                                                Rectangle {
                                                    width: 18
                                                    height: 18
                                                    color: "#f0f2f5"
                                                    border.color: "#bfc6d0"
                                                    Text { anchors.centerIn: parent; text: "▶"; font.pixelSize: 10; color: "#3f4753" }
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            AppPanel {
                                Layout.preferredWidth: 338
                                Layout.fillHeight: true
                                color: ThemeLib.Theme.color.panelSubtleBg

                                AppTabHeader {
                                    id: tuneTabHeader
                                    width: parent.width
                                    height: 30
                                    labels: ["参数配置", "仪器状态", "设备连接"]
                                    currentIndex: tuneRightStack.currentIndex
                                    onSelected: function(index) { tuneRightStack.currentIndex = index }
                                }

                                StackLayout {
                                    id: tuneRightStack
                                    anchors.top: tuneTabHeader.bottom
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.bottom: parent.bottom

                                    ScrollView {
                                        clip: true
                                        ColumnLayout {
                                            x: 8
                                            width: Math.max(0, parent.width - 16)
                                            spacing: 6

                                            AppCheckBox { text: "显示高级参数" }

                                            GroupBox {
                                                title: "离子源"
                                                Layout.fillWidth: true
                                                GridLayout {
                                                    columns: 3
                                                    Label { text: "推斥极 [V]"; font.pixelSize: 12 }
                                                    SpinBox {
                                                        id: repeller
                                                        from: -200
                                                        to: 200
                                                        value: 12
                                                        editable: true
                                                        implicitWidth: 120
                                                        implicitHeight: 24
                                                        leftPadding: 20
                                                        rightPadding: 20
                                                        background: Rectangle { color: "#f3f4f6"; border.color: "#cfd4dc"; radius: 2 }
                                                        down.indicator: Rectangle {
                                                            x: 0
                                                            y: 0
                                                            width: 18
                                                            height: repeller.height
                                                            color: "#eceff3"
                                                            border.color: "#cfd4dc"
                                                            Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 12; color: "#344054" }
                                                        }
                                                        up.indicator: Rectangle {
                                                            x: repeller.width - width
                                                            y: 0
                                                            width: 18
                                                            height: repeller.height
                                                            color: "#eceff3"
                                                            border.color: "#cfd4dc"
                                                            Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 12; color: "#344054" }
                                                        }
                                                        onValueChanged: {
                                                            if (repellerSlider.value !== value) repellerSlider.value = value
                                                        }
                                                    }
                                                    Slider {
                                                        id: repellerSlider
                                                        from: -200
                                                        to: 200
                                                        value: repeller.value
                                                        onMoved: repeller.value = value
                                                        onValueChanged: {
                                                            if (repeller.value !== value) repeller.value = value
                                                        }
                                                        implicitHeight: 20
                                                        Layout.fillWidth: true
                                                        Layout.preferredWidth: 120
                                                        background: Rectangle {
                                                            x: parent.leftPadding
                                                            y: parent.topPadding + parent.availableHeight / 2 - 3
                                                            width: parent.availableWidth
                                                            height: 6
                                                            radius: 3
                                                            color: "#e5e7eb"
                                                            Rectangle {
                                                                width: parent.width * ((parent.parent.value - parent.parent.from) / (parent.parent.to - parent.parent.from))
                                                                height: parent.height
                                                                radius: 3
                                                                color: "#4b5563"
                                                            }
                                                        }
                                                        handle: Rectangle {
                                                            x: parent.leftPadding + (parent.availableWidth - width) * parent.position
                                                            y: parent.topPadding + parent.availableHeight / 2 - height / 2
                                                            width: 16
                                                            height: 16
                                                            radius: 8
                                                            color: "#f5f6f7"
                                                            border.color: "#c8ccd2"
                                                        }
                                                    }
                                                    Label { text: "透镜1(选出) [V]"; font.pixelSize: 12 }
                                                    SpinBox {
                                                        id: lens1
                                                        from: -200
                                                        to: 200
                                                        value: 6
                                                        editable: true
                                                        implicitWidth: 120
                                                        implicitHeight: 24
                                                        leftPadding: 20
                                                        rightPadding: 20
                                                        background: Rectangle { color: "#f3f4f6"; border.color: "#cfd4dc"; radius: 2 }
                                                        down.indicator: Rectangle {
                                                            x: 0
                                                            y: 0
                                                            width: 18
                                                            height: lens1.height
                                                            color: "#eceff3"
                                                            border.color: "#cfd4dc"
                                                            Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 12; color: "#344054" }
                                                        }
                                                        up.indicator: Rectangle {
                                                            x: lens1.width - width
                                                            y: 0
                                                            width: 18
                                                            height: lens1.height
                                                            color: "#eceff3"
                                                            border.color: "#cfd4dc"
                                                            Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 12; color: "#344054" }
                                                        }
                                                        onValueChanged: {
                                                            if (lens1Slider.value !== value) lens1Slider.value = value
                                                        }
                                                    }
                                                    Slider {
                                                        id: lens1Slider
                                                        from: -200
                                                        to: 200
                                                        value: lens1.value
                                                        onMoved: lens1.value = value
                                                        onValueChanged: {
                                                            if (lens1.value !== value) lens1.value = value
                                                        }
                                                        implicitHeight: 20
                                                        Layout.fillWidth: true
                                                        Layout.preferredWidth: 120
                                                        background: Rectangle {
                                                            x: parent.leftPadding
                                                            y: parent.topPadding + parent.availableHeight / 2 - 3
                                                            width: parent.availableWidth
                                                            height: 6
                                                            radius: 3
                                                            color: "#e5e7eb"
                                                            Rectangle {
                                                                width: parent.width * ((parent.parent.value - parent.parent.from) / (parent.parent.to - parent.parent.from))
                                                                height: parent.height
                                                                radius: 3
                                                                color: "#4b5563"
                                                            }
                                                        }
                                                        handle: Rectangle {
                                                            x: parent.leftPadding + (parent.availableWidth - width) * parent.position
                                                            y: parent.topPadding + parent.availableHeight / 2 - height / 2
                                                            width: 16
                                                            height: 16
                                                            radius: 8
                                                            color: "#f5f6f7"
                                                            border.color: "#c8ccd2"
                                                        }
                                                    }
                                                    Label { text: "透镜2(聚焦) [V]"; font.pixelSize: 12 }
                                                    SpinBox {
                                                        id: lens2
                                                        from: -200
                                                        to: 200
                                                        value: 10
                                                        editable: true
                                                        implicitWidth: 120
                                                        implicitHeight: 24
                                                        leftPadding: 20
                                                        rightPadding: 20
                                                        background: Rectangle { color: "#f3f4f6"; border.color: "#cfd4dc"; radius: 2 }
                                                        down.indicator: Rectangle {
                                                            x: 0
                                                            y: 0
                                                            width: 18
                                                            height: lens2.height
                                                            color: "#eceff3"
                                                            border.color: "#cfd4dc"
                                                            Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 12; color: "#344054" }
                                                        }
                                                        up.indicator: Rectangle {
                                                            x: lens2.width - width
                                                            y: 0
                                                            width: 18
                                                            height: lens2.height
                                                            color: "#eceff3"
                                                            border.color: "#cfd4dc"
                                                            Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 12; color: "#344054" }
                                                        }
                                                        onValueChanged: {
                                                            if (lens2Slider.value !== value) lens2Slider.value = value
                                                        }
                                                    }
                                                    Slider {
                                                        id: lens2Slider
                                                        from: -200
                                                        to: 200
                                                        value: lens2.value
                                                        onMoved: lens2.value = value
                                                        onValueChanged: {
                                                            if (lens2.value !== value) lens2.value = value
                                                        }
                                                        implicitHeight: 20
                                                        Layout.fillWidth: true
                                                        Layout.preferredWidth: 120
                                                        background: Rectangle {
                                                            x: parent.leftPadding
                                                            y: parent.topPadding + parent.availableHeight / 2 - 3
                                                            width: parent.availableWidth
                                                            height: 6
                                                            radius: 3
                                                            color: "#e5e7eb"
                                                            Rectangle {
                                                                width: parent.width * ((parent.parent.value - parent.parent.from) / (parent.parent.to - parent.parent.from))
                                                                height: parent.height
                                                                radius: 3
                                                                color: "#4b5563"
                                                            }
                                                        }
                                                        handle: Rectangle {
                                                            x: parent.leftPadding + (parent.availableWidth - width) * parent.position
                                                            y: parent.topPadding + parent.availableHeight / 2 - height / 2
                                                            width: 16
                                                            height: 16
                                                            radius: 8
                                                            color: "#f5f6f7"
                                                            border.color: "#c8ccd2"
                                                        }
                                                    }
                                                }
                                            }

                                            GroupBox {
                                                title: "检测器"
                                                Layout.fillWidth: true
                                                GridLayout {
                                                    columns: 3
                                                    Label { text: "高端补偿"; font.pixelSize: 12 }
                                                    SpinBox {
                                                        id: highComp
                                                        from: -100
                                                        to: 100
                                                        value: 0
                                                        editable: true
                                                        implicitWidth: 120
                                                        implicitHeight: 24
                                                        leftPadding: 20
                                                        rightPadding: 20
                                                        background: Rectangle { color: "#f3f4f6"; border.color: "#cfd4dc"; radius: 2 }
                                                        down.indicator: Rectangle {
                                                            x: 0
                                                            y: 0
                                                            width: 18
                                                            height: highComp.height
                                                            color: "#eceff3"
                                                            border.color: "#cfd4dc"
                                                            Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 12; color: "#344054" }
                                                        }
                                                        up.indicator: Rectangle {
                                                            x: highComp.width - width
                                                            y: 0
                                                            width: 18
                                                            height: highComp.height
                                                            color: "#eceff3"
                                                            border.color: "#cfd4dc"
                                                            Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 12; color: "#344054" }
                                                        }
                                                        onValueChanged: {
                                                            if (highCompSlider.value !== value) highCompSlider.value = value
                                                        }
                                                    }
                                                    Slider {
                                                        id: highCompSlider
                                                        from: -100
                                                        to: 100
                                                        value: highComp.value
                                                        onMoved: highComp.value = value
                                                        onValueChanged: {
                                                            if (highComp.value !== value) highComp.value = value
                                                        }
                                                        implicitHeight: 20
                                                        Layout.fillWidth: true
                                                        Layout.preferredWidth: 120
                                                        background: Rectangle {
                                                            x: parent.leftPadding
                                                            y: parent.topPadding + parent.availableHeight / 2 - 3
                                                            width: parent.availableWidth
                                                            height: 6
                                                            radius: 3
                                                            color: "#e5e7eb"
                                                            Rectangle {
                                                                width: parent.width * ((parent.parent.value - parent.parent.from) / (parent.parent.to - parent.parent.from))
                                                                height: parent.height
                                                                radius: 3
                                                                color: "#4b5563"
                                                            }
                                                        }
                                                        handle: Rectangle {
                                                            x: parent.leftPadding + (parent.availableWidth - width) * parent.position
                                                            y: parent.topPadding + parent.availableHeight / 2 - height / 2
                                                            width: 16
                                                            height: 16
                                                            radius: 8
                                                            color: "#f5f6f7"
                                                            border.color: "#c8ccd2"
                                                        }
                                                    }
                                                    Label { text: "低端补偿"; font.pixelSize: 12 }
                                                    SpinBox {
                                                        id: lowComp
                                                        from: -100
                                                        to: 100
                                                        value: 0
                                                        editable: true
                                                        implicitWidth: 120
                                                        implicitHeight: 24
                                                        leftPadding: 20
                                                        rightPadding: 20
                                                        background: Rectangle { color: "#f3f4f6"; border.color: "#cfd4dc"; radius: 2 }
                                                        down.indicator: Rectangle {
                                                            x: 0
                                                            y: 0
                                                            width: 18
                                                            height: lowComp.height
                                                            color: "#eceff3"
                                                            border.color: "#cfd4dc"
                                                            Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 12; color: "#344054" }
                                                        }
                                                        up.indicator: Rectangle {
                                                            x: lowComp.width - width
                                                            y: 0
                                                            width: 18
                                                            height: lowComp.height
                                                            color: "#eceff3"
                                                            border.color: "#cfd4dc"
                                                            Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 12; color: "#344054" }
                                                        }
                                                        onValueChanged: {
                                                            if (lowCompSlider.value !== value) lowCompSlider.value = value
                                                        }
                                                    }
                                                    Slider {
                                                        id: lowCompSlider
                                                        from: -100
                                                        to: 100
                                                        value: lowComp.value
                                                        onMoved: lowComp.value = value
                                                        onValueChanged: {
                                                            if (lowComp.value !== value) lowComp.value = value
                                                        }
                                                        implicitHeight: 20
                                                        Layout.fillWidth: true
                                                        Layout.preferredWidth: 120
                                                        background: Rectangle {
                                                            x: parent.leftPadding
                                                            y: parent.topPadding + parent.availableHeight / 2 - 3
                                                            width: parent.availableWidth
                                                            height: 6
                                                            radius: 3
                                                            color: "#e5e7eb"
                                                            Rectangle {
                                                                width: parent.width * ((parent.parent.value - parent.parent.from) / (parent.parent.to - parent.parent.from))
                                                                height: parent.height
                                                                radius: 3
                                                                color: "#4b5563"
                                                            }
                                                        }
                                                        handle: Rectangle {
                                                            x: parent.leftPadding + (parent.availableWidth - width) * parent.position
                                                            y: parent.topPadding + parent.availableHeight / 2 - height / 2
                                                            width: 16
                                                            height: 16
                                                            radius: 8
                                                            color: "#f5f6f7"
                                                            border.color: "#c8ccd2"
                                                        }
                                                    }
                                                    Label { text: "倍增器 [V]"; font.pixelSize: 12 }
                                                    SpinBox {
                                                        id: multiplierVoltage
                                                        from: 0
                                                        to: 3000
                                                        value: 1200
                                                        editable: true
                                                        implicitWidth: 120
                                                        implicitHeight: 24
                                                        leftPadding: 20
                                                        rightPadding: 20
                                                        background: Rectangle { color: "#f3f4f6"; border.color: "#cfd4dc"; radius: 2 }
                                                        down.indicator: Rectangle {
                                                            x: 0
                                                            y: 0
                                                            width: 18
                                                            height: multiplierVoltage.height
                                                            color: "#eceff3"
                                                            border.color: "#cfd4dc"
                                                            Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 12; color: "#344054" }
                                                        }
                                                        up.indicator: Rectangle {
                                                            x: multiplierVoltage.width - width
                                                            y: 0
                                                            width: 18
                                                            height: multiplierVoltage.height
                                                            color: "#eceff3"
                                                            border.color: "#cfd4dc"
                                                            Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 12; color: "#344054" }
                                                        }
                                                        onValueChanged: {
                                                            if (multiplierSlider.value !== value) multiplierSlider.value = value
                                                        }
                                                    }
                                                    Slider {
                                                        id: multiplierSlider
                                                        from: 0
                                                        to: 3000
                                                        value: multiplierVoltage.value
                                                        onMoved: multiplierVoltage.value = value
                                                        onValueChanged: {
                                                            if (multiplierVoltage.value !== value) multiplierVoltage.value = value
                                                        }
                                                        implicitHeight: 20
                                                        Layout.fillWidth: true
                                                        Layout.preferredWidth: 120
                                                        background: Rectangle {
                                                            x: parent.leftPadding
                                                            y: parent.topPadding + parent.availableHeight / 2 - 3
                                                            width: parent.availableWidth
                                                            height: 6
                                                            radius: 3
                                                            color: "#e5e7eb"
                                                            Rectangle {
                                                                width: parent.width * ((parent.parent.value - parent.parent.from) / (parent.parent.to - parent.parent.from))
                                                                height: parent.height
                                                                radius: 3
                                                                color: "#4b5563"
                                                            }
                                                        }
                                                        handle: Rectangle {
                                                            x: parent.leftPadding + (parent.availableWidth - width) * parent.position
                                                            y: parent.topPadding + parent.availableHeight / 2 - height / 2
                                                            width: 16
                                                            height: 16
                                                            radius: 8
                                                            color: "#f5f6f7"
                                                            border.color: "#c8ccd2"
                                                        }
                                                    }
                                                }
                                            }

                                            AppButton {
                                                text: "应用"
                                                implicitWidth: 70
                                                onClicked: appState.applyTune(repeller.value, lens1.value, lens2.value)
                                            }
                                        }
                                    }

                                    ScrollView {
                                        clip: true
                                        ColumnLayout {
                                            width: parent.width
                                            spacing: 10
                                            anchors.margins: 8

                                            GroupBox {
                                                title: "部件温度[℃]"
                                                Layout.fillWidth: true
                                                RowLayout {
                                                    anchors.fill: parent
                                                    anchors.margins: 10
                                                    spacing: 8
                                                    Repeater {
                                                        model: ["进样管路1", "进样管路2", "进样腔体", "真空腔体"]
                                                        delegate: ColumnLayout {
                                                            spacing: 4
                                                            Label {
                                                                Layout.fillWidth: true
                                                                horizontalAlignment: Text.AlignHCenter
                                                                text: "0"
                                                                font.pixelSize: 13
                                                                font.bold: true
                                                                color: "#344054"
                                                            }
                                                            Rectangle {
                                                                Layout.alignment: Qt.AlignHCenter
                                                                width: 20
                                                                height: 156
                                                                radius: 3
                                                                border.color: "#c8cdd6"
                                                                color: "#f7f8fa"
                                                                Rectangle {
                                                                    anchors.left: parent.left
                                                                    anchors.right: parent.right
                                                                    anchors.bottom: parent.bottom
                                                                    height: appState.connected ? 36 : 0
                                                                    radius: 2
                                                                    color: "#57a5ff"
                                                                }
                                                            }
                                                            Label {
                                                                Layout.fillWidth: true
                                                                horizontalAlignment: Text.AlignHCenter
                                                                text: "100"
                                                                font.pixelSize: 12
                                                                color: "#98a2b3"
                                                            }
                                                            Label {
                                                                Layout.fillWidth: true
                                                                horizontalAlignment: Text.AlignHCenter
                                                                text: modelData
                                                                wrapMode: Text.WordWrap
                                                                font.pixelSize: 12
                                                                color: "#475467"
                                                            }
                                                        }
                                                    }
                                                }
                                            }

                                            GroupBox {
                                                title: "部件状态"
                                                Layout.fillWidth: true

                                                ColumnLayout {
                                                    width: parent.width
                                                    spacing: 0
                                                    property var names: [
                                                        "+24V", "灯丝电流[A]", "电子能量[eV]", "发射电流[μA]", "射频扫描电压[V]", "射频电流[A]",
                                                        "倍增器电压[V]", "前预四极[V]", "打拿极[V]", "推斥极[V]", "透镜1[V]", "透镜2[V]"
                                                    ]
                                                    property var values: [
                                                        appState.connected ? "24" : "0",
                                                        "0.0", "0.0", "0.0",
                                                        "100.0", "20.0",
                                                        "1200.0", "0.0", "0.0", "0.0", "0.0", "0.0"
                                                    ]

                                                    Repeater {
                                                        model: 12
                                                        delegate: RowLayout {
                                                            Layout.fillWidth: true
                                                            Layout.preferredHeight: 28
                                                            spacing: 0
                                                            Rectangle {
                                                                Layout.fillWidth: true
                                                                Layout.preferredWidth: 1.45
                                                                Layout.fillHeight: true
                                                                border.color: "#e4e7ec"
                                                                color: index % 2 === 1 ? "#f8fbff" : "#ffffff"
                                                                Text {
                                                                    anchors.left: parent.left
                                                                    anchors.leftMargin: 8
                                                                    anchors.verticalCenter: parent.verticalCenter
                                                                    text: parent.parent.parent.names[index]
                                                                    font.pixelSize: 12
                                                                    color: "#344054"
                                                                }
                                                            }
                                                            Rectangle {
                                                                Layout.fillWidth: true
                                                                Layout.preferredWidth: 1.0
                                                                Layout.fillHeight: true
                                                                border.color: "#e4e7ec"
                                                                color: index % 2 === 1 ? "#f8fbff" : "#ffffff"
                                                                Text {
                                                                    anchors.centerIn: parent
                                                                    text: parent.parent.parent.values[index]
                                                                    font.pixelSize: 12
                                                                    color: "#344054"
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    Rectangle {
                                        color: "#f6f6f7"
                                        border.color: "#c9ccd2"
                                        radius: 2

                                        ColumnLayout {
                                            anchors.fill: parent
                                            anchors.margins: 10
                                            spacing: 8

                                            Label {
                                                text: "IP地址"
                                                font.pixelSize: 12
                                                color: "#344054"
                                            }
                                            TextField {
                                                id: tuneHostField
                                                Layout.fillWidth: true
                                                implicitHeight: 28
                                                font.pixelSize: 13
                                                topPadding: 3
                                                bottomPadding: 3
                                                text: appState.connectionHost
                                            }

                                            Label {
                                                text: "端口号"
                                                font.pixelSize: 12
                                                color: "#344054"
                                            }
                                            SpinBox {
                                                id: tunePortField
                                                Layout.fillWidth: true
                                                from: 1
                                                to: 65535
                                                value: appState.connectionPort
                                                editable: true
                                                implicitHeight: 30
                                            }

                                            RowLayout {
                                                Layout.fillWidth: true
                                                spacing: 10
                                                AppButton {
                                                    text: "建立连接"
                                                    implicitWidth: 84
                                                    implicitHeight: 28
                                                    onClicked: appState.connectToDevice(tuneHostField.text, tunePortField.value)
                                                }
                                                AppButton {
                                                    text: "断开连接"
                                                    implicitWidth: 84
                                                    implicitHeight: 28
                                                    onClicked: appState.disconnectFromDevice()
                                                }
                                                Item { Layout.fillWidth: true }
                                            }

                                            Label {
                                                text: "状态: " + (appState.connected ? "已连接" : "未连接")
                                                font.pixelSize: 12
                                                color: appState.connected ? "#027a48" : "#475467"
                                            }
                                            Item { Layout.fillHeight: true }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Item {
                    id: monitorPage
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 6

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 31
                            color: ThemeLib.Theme.color.toolbarBg
                            border.color: ThemeLib.Theme.color.borderStrong

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 3
                                spacing: 4
                                AppButton { text: "打开方法"; implicitWidth: 66 }
                                AppButton { text: "运行方法"; implicitWidth: 66; onClicked: appState.startScan(10, 110) }
                                AppButton { text: "暂停刷新"; implicitWidth: 66 }
                                AppButton { text: "数据另存"; implicitWidth: 66 }
                                Item { Layout.fillWidth: true }
                                Label {
                                    text: appState.connected ? (appState.scanning ? "扫描运行中" : "设备空闲") : "设备未连接"
                                    color: ThemeLib.Theme.color.textSecondary
                                    font.pixelSize: 12
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: 8

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                color: "#ffffff"
                                border.color: ThemeLib.Theme.color.borderDefault

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 4
                                    spacing: 4

                                    ChartView {
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true
                                        Layout.preferredHeight: 1.2
                                        antialiasing: true
                                        legend.visible: false
                                        title: "RIC"
                                        backgroundColor: ThemeLib.Theme.color.chartBg
                                        plotAreaColor: ThemeLib.Theme.color.chartBg
                                        ValueAxis { id: monitorRx; min: 0; max: 240 }
                                        ValueAxis { id: monitorRy; min: 0; max: 2000 }
                                        LineSeries {
                                            id: monitorRicSeries
                                            axisX: monitorRx
                                            axisY: monitorRy
                                            color: ThemeLib.Theme.color.linePrimary
                                            width: 2
                                        }
                                        Component.onCompleted: appState.bindRicSeries(monitorRicSeries)
                                    }

                                    ChartView {
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true
                                        Layout.preferredHeight: 1.2
                                        antialiasing: true
                                        legend.visible: false
                                        title: "TIC"
                                        backgroundColor: ThemeLib.Theme.color.chartBg
                                        plotAreaColor: ThemeLib.Theme.color.chartBg
                                        ValueAxis { id: monitorTx; min: 0; max: 240 }
                                        ValueAxis { id: monitorTy; min: 0; max: 250000 }
                                        LineSeries {
                                            id: monitorTicSeries
                                            axisX: monitorTx
                                            axisY: monitorTy
                                            color: ThemeLib.Theme.color.linePrimary
                                            width: 2
                                        }
                                        Component.onCompleted: appState.bindTicSeries(monitorTicSeries)
                                    }
                                }
                            }

                            AppPanel {
                                Layout.preferredWidth: 420
                                Layout.fillHeight: true
                                color: ThemeLib.Theme.color.panelSubtleBg

                                AppTabHeader {
                                    id: monitorTabHeader
                                    width: parent.width
                                    height: 30
                                    labels: ["参数配置", "仪器状态"]
                                    currentIndex: monitorRightStack.currentIndex
                                    onSelected: function(index) { monitorRightStack.currentIndex = index }
                                }

                                StackLayout {
                                    id: monitorRightStack
                                    anchors.top: monitorTabHeader.bottom
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.bottom: parent.bottom

                                    ScrollView {
                                        clip: true
                                        ColumnLayout {
                                            width: parent.width
                                            spacing: 6
                                            anchors.margins: 6

                                            GroupBox {
                                                title: "检测器"
                                                Layout.fillWidth: true
                                                RowLayout {
                                                    AppCheckBox { text: "倍增器"; checked: true }
                                                    AppCheckBox { text: "法拉第筒" }
                                                }
                                            }

                                            GroupBox {
                                                title: "稳定区"
                                                Layout.fillWidth: true
                                                RowLayout {
                                                    AppCheckBox { text: "稳定区一"; checked: true }
                                                    AppCheckBox { text: "稳定区二" }
                                                }
                                            }

                                            GroupBox {
                                                title: "扫描模式"
                                                Layout.fillWidth: true
                                                RowLayout {
                                                    AppCheckBox { text: "Full Scan"; checked: true }
                                                    AppCheckBox { text: "SIM" }
                                                }
                                            }

                                            GroupBox {
                                                title: "SIM 参数表"
                                                Layout.fillWidth: true

                                                ColumnLayout {
                                                    width: parent.width
                                                    spacing: 0

                                                    RowLayout {
                                                        Layout.fillWidth: true
                                                        spacing: 0
                                                        Rectangle {
                                                            Layout.preferredWidth: 52
                                                            Layout.preferredHeight: 28
                                                            border.color: "#d8dde5"
                                                            color: "#f6f8fb"
                                                            Text { anchors.centerIn: parent; text: "启用"; font.pixelSize: 12; font.bold: true; color: ThemeLib.Theme.color.textPrimary }
                                                        }
                                                        Rectangle {
                                                            Layout.preferredWidth: 74
                                                            Layout.preferredHeight: 28
                                                            border.color: "#d8dde5"
                                                            color: "#f6f8fb"
                                                            Text { anchors.centerIn: parent; text: "质量数"; font.pixelSize: 12; font.bold: true; color: ThemeLib.Theme.color.textPrimary }
                                                        }
                                                        Rectangle {
                                                            Layout.preferredWidth: 78
                                                            Layout.preferredHeight: 28
                                                            border.color: "#d8dde5"
                                                            color: "#f6f8fb"
                                                            Text { anchors.centerIn: parent; text: "停留(ms)"; font.pixelSize: 12; font.bold: true; color: ThemeLib.Theme.color.textPrimary }
                                                        }
                                                        Rectangle {
                                                            Layout.preferredWidth: 64
                                                            Layout.preferredHeight: 28
                                                            border.color: "#d8dde5"
                                                            color: "#f6f8fb"
                                                            Text { anchors.centerIn: parent; text: "最小"; font.pixelSize: 12; font.bold: true; color: ThemeLib.Theme.color.textPrimary }
                                                        }
                                                        Rectangle {
                                                            Layout.preferredWidth: 64
                                                            Layout.preferredHeight: 28
                                                            border.color: "#d8dde5"
                                                            color: "#f6f8fb"
                                                            Text { anchors.centerIn: parent; text: "最大"; font.pixelSize: 12; font.bold: true; color: ThemeLib.Theme.color.textPrimary }
                                                        }
                                                        Item { Layout.fillWidth: true }
                                                    }

                                                    Repeater {
                                                        model: simTableModel
                                                        delegate: RowLayout {
                                                            Layout.fillWidth: true
                                                            spacing: 0

                                                            Rectangle {
                                                                Layout.preferredWidth: 52
                                                                Layout.preferredHeight: 30
                                                                border.color: "#d8dde5"
                                                                color: index === root.monitorSelectedRow ? "#eef5ff" : "#ffffff"
                                                                AppCheckBox {
                                                                    anchors.centerIn: parent
                                                                    checked: model.enabled
                                                                    text: ""
                                                                    onToggled: simTableModel.setProperty(index, "enabled", checked)
                                                                }
                                                                MouseArea {
                                                                    anchors.fill: parent
                                                                    onClicked: root.monitorSelectedRow = index
                                                                }
                                                            }

                                                            Rectangle {
                                                                Layout.preferredWidth: 74
                                                                Layout.preferredHeight: 30
                                                                border.color: "#d8dde5"
                                                                color: index === root.monitorSelectedRow ? "#eef5ff" : "#ffffff"
                                                                TextField {
                                                                    anchors.fill: parent
                                                                    anchors.margins: 2
                                                                    text: model.mass
                                                                    horizontalAlignment: TextInput.AlignHCenter
                                                                    verticalAlignment: TextInput.AlignVCenter
                                                                    background: Rectangle { color: "transparent" }
                                                                    onEditingFinished: simTableModel.setProperty(index, "mass", text)
                                                                }
                                                                MouseArea {
                                                                    anchors.fill: parent
                                                                    onClicked: root.monitorSelectedRow = index
                                                                }
                                                            }

                                                            Rectangle {
                                                                Layout.preferredWidth: 78
                                                                Layout.preferredHeight: 30
                                                                border.color: "#d8dde5"
                                                                color: index === root.monitorSelectedRow ? "#eef5ff" : "#ffffff"
                                                                TextField {
                                                                    anchors.fill: parent
                                                                    anchors.margins: 2
                                                                    text: model.dwell
                                                                    horizontalAlignment: TextInput.AlignHCenter
                                                                    verticalAlignment: TextInput.AlignVCenter
                                                                    background: Rectangle { color: "transparent" }
                                                                    onEditingFinished: simTableModel.setProperty(index, "dwell", text)
                                                                }
                                                                MouseArea {
                                                                    anchors.fill: parent
                                                                    onClicked: root.monitorSelectedRow = index
                                                                }
                                                            }

                                                            Rectangle {
                                                                Layout.preferredWidth: 64
                                                                Layout.preferredHeight: 30
                                                                border.color: "#d8dde5"
                                                                color: index === root.monitorSelectedRow ? "#eef5ff" : "#ffffff"
                                                                TextField {
                                                                    anchors.fill: parent
                                                                    anchors.margins: 2
                                                                    text: model.minv
                                                                    horizontalAlignment: TextInput.AlignHCenter
                                                                    verticalAlignment: TextInput.AlignVCenter
                                                                    background: Rectangle { color: "transparent" }
                                                                    onEditingFinished: simTableModel.setProperty(index, "minv", text)
                                                                }
                                                                MouseArea {
                                                                    anchors.fill: parent
                                                                    onClicked: root.monitorSelectedRow = index
                                                                }
                                                            }

                                                            Rectangle {
                                                                Layout.preferredWidth: 64
                                                                Layout.preferredHeight: 30
                                                                border.color: "#d8dde5"
                                                                color: index === root.monitorSelectedRow ? "#eef5ff" : "#ffffff"
                                                                TextField {
                                                                    anchors.fill: parent
                                                                    anchors.margins: 2
                                                                    text: model.maxv
                                                                    horizontalAlignment: TextInput.AlignHCenter
                                                                    verticalAlignment: TextInput.AlignVCenter
                                                                    background: Rectangle { color: "transparent" }
                                                                    onEditingFinished: simTableModel.setProperty(index, "maxv", text)
                                                                }
                                                                MouseArea {
                                                                    anchors.fill: parent
                                                                    onClicked: root.monitorSelectedRow = index
                                                                }
                                                            }
                                                            Item { Layout.fillWidth: true }
                                                        }
                                                    }
                                                }
                                            }

                                            RowLayout {
                                                Layout.fillWidth: true
                                                spacing: 6
                                                AppButton {
                                                    text: "删除选中"
                                                    implicitWidth: 72
                                                    onClicked: {
                                                        if (root.monitorSelectedRow >= 0 && root.monitorSelectedRow < simTableModel.count) {
                                                            simTableModel.remove(root.monitorSelectedRow, 1)
                                                            root.monitorSelectedRow = -1
                                                        }
                                                    }
                                                }
                                                AppButton {
                                                    text: "设置"
                                                    implicitWidth: 52
                                                    onClicked: appState.toastRequested("SIM 参数已更新")
                                                }
                                                Item { Layout.fillWidth: true }
                                            }
                                        }
                                    }

                                    ScrollView {
                                        clip: true
                                        ColumnLayout {
                                            width: parent.width
                                            spacing: 10
                                            anchors.margins: 8

                                            GroupBox {
                                                title: "部件温度[℃]"
                                                Layout.fillWidth: true
                                                RowLayout {
                                                    anchors.fill: parent
                                                    anchors.margins: 10
                                                    spacing: 8
                                                    Repeater {
                                                        model: ["进样管路1", "进样管路2", "进样腔体", "真空腔体"]
                                                        delegate: ColumnLayout {
                                                            spacing: 4
                                                            Label {
                                                                Layout.fillWidth: true
                                                                horizontalAlignment: Text.AlignHCenter
                                                                text: "0"
                                                                font.pixelSize: 13
                                                                font.bold: true
                                                                color: "#344054"
                                                            }
                                                            Rectangle {
                                                                Layout.alignment: Qt.AlignHCenter
                                                                width: 20
                                                                height: 156
                                                                radius: 3
                                                                border.color: "#c8cdd6"
                                                                color: "#f7f8fa"
                                                                Rectangle {
                                                                    anchors.left: parent.left
                                                                    anchors.right: parent.right
                                                                    anchors.bottom: parent.bottom
                                                                    height: appState.connected ? 36 : 0
                                                                    radius: 2
                                                                    color: "#57a5ff"
                                                                }
                                                            }
                                                            Label {
                                                                Layout.fillWidth: true
                                                                horizontalAlignment: Text.AlignHCenter
                                                                text: "100"
                                                                font.pixelSize: 12
                                                                color: "#98a2b3"
                                                            }
                                                            Label {
                                                                Layout.fillWidth: true
                                                                horizontalAlignment: Text.AlignHCenter
                                                                text: modelData
                                                                wrapMode: Text.WordWrap
                                                                font.pixelSize: 12
                                                                color: "#475467"
                                                            }
                                                        }
                                                    }
                                                }
                                            }

                                            GroupBox {
                                                title: "部件状态"
                                                Layout.fillWidth: true

                                                ColumnLayout {
                                                    width: parent.width
                                                    spacing: 0
                                                    property var names: [
                                                        "+24V", "灯丝电流[A]", "电子能量[eV]", "发射电流[μA]", "射频扫描电压[V]", "射频电流[A]",
                                                        "倍增器电压[V]", "前预四极[V]", "打拿极[V]", "推斥极[V]", "透镜1[V]", "透镜2[V]"
                                                    ]
                                                    property var values: [
                                                        appState.connected ? "24" : "0",
                                                        "0.0", "0.0", "0.0",
                                                        "100.0", "20.0",
                                                        "1200.0", "0.0", "0.0", "0.0", "0.0", "0.0"
                                                    ]

                                                    Repeater {
                                                        model: 12
                                                        delegate: RowLayout {
                                                            Layout.fillWidth: true
                                                            Layout.preferredHeight: 28
                                                            spacing: 0
                                                            Rectangle {
                                                                Layout.fillWidth: true
                                                                Layout.preferredWidth: 1.45
                                                                Layout.fillHeight: true
                                                                border.color: "#e4e7ec"
                                                                color: index % 2 === 1 ? "#f8fbff" : "#ffffff"
                                                                Text {
                                                                    anchors.left: parent.left
                                                                    anchors.leftMargin: 8
                                                                    anchors.verticalCenter: parent.verticalCenter
                                                                    text: parent.parent.parent.names[index]
                                                                    font.pixelSize: 12
                                                                    color: "#344054"
                                                                }
                                                            }
                                                            Rectangle {
                                                                Layout.fillWidth: true
                                                                Layout.preferredWidth: 1.0
                                                                Layout.fillHeight: true
                                                                border.color: "#e4e7ec"
                                                                color: index % 2 === 1 ? "#f8fbff" : "#ffffff"
                                                                Text {
                                                                    anchors.centerIn: parent
                                                                    text: parent.parent.parent.values[index]
                                                                    font.pixelSize: 12
                                                                    color: "#344054"
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 28
                            color: "#e9edf3"
                            border.color: ThemeLib.Theme.color.borderStrong

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 6
                                anchors.rightMargin: 6
                                AppButton {
                                    text: appState.scanning ? "停止扫描" : "开始扫描"
                                    implicitWidth: 72
                                    onClicked: appState.scanning ? appState.stopScan() : appState.startScan(10, 110)
                                }
                                Item { Layout.fillWidth: true }
                                Label {
                                    text: "真空度: " + f2(appState.vacuumPa) + " Pa"
                                    font.pixelSize: 12
                                    color: ThemeLib.Theme.color.textSecondary
                                }
                            }
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Rectangle {
                        anchors.fill: parent
                        color: "#ffffff"
                        border.color: ThemeLib.Theme.color.borderDefault
                        Text {
                            anchors.centerIn: parent
                            text: "方法页开发中"
                            color: ThemeLib.Theme.color.textSecondary
                            font.pixelSize: 14
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 0

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 36
                            color: "#c8c3e8"
                            border.color: "#b4aed9"

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                anchors.rightMargin: 10
                                anchors.topMargin: 4
                                anchors.bottomMargin: 4

                                AppButton {
                                    text: "打开数据"
                                    implicitWidth: 96
                                    implicitHeight: 28
                                }
                                Item { Layout.fillWidth: true }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.topMargin: 4
                            spacing: 0

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                color: "#f7f7f7"
                                border.color: "#a9adb7"

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 2
                                    spacing: 2

                                    ChartView {
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true
                                        Layout.preferredHeight: 5
                                        antialiasing: true
                                        legend.visible: false
                                        title: "全扫描"
                                        backgroundColor: "#f4f5f7"
                                        plotAreaColor: "#f4f5f7"
                                        ValueAxis { id: dataSx; min: 0; max: 620 }
                                        ValueAxis { id: dataSy; min: 0; max: 1000 }
                                        LineSeries {
                                            axisX: dataSx
                                            axisY: dataSy
                                            color: ThemeLib.Theme.color.linePrimary
                                            width: 2
                                            XYPoint { x: 0; y: 0 }
                                            XYPoint { x: 620; y: 0 }
                                        }
                                    }

                                    ChartView {
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true
                                        Layout.preferredHeight: 2
                                        antialiasing: true
                                        legend.visible: false
                                        title: "TIC"
                                        backgroundColor: "#f4f5f7"
                                        plotAreaColor: "#f4f5f7"
                                        ValueAxis { id: dataTx; min: 0; max: 1000 }
                                        ValueAxis { id: dataTy; min: 0; max: 1000 }
                                        LineSeries {
                                            axisX: dataTx
                                            axisY: dataTy
                                            color: ThemeLib.Theme.color.linePrimary
                                            width: 2
                                            XYPoint { x: 0; y: 0 }
                                            XYPoint { x: 1000; y: 0 }
                                        }
                                    }
                                }
                            }

                            Rectangle {
                                Layout.preferredWidth: 258
                                Layout.minimumWidth: 258
                                Layout.maximumWidth: 258
                                Layout.fillHeight: true
                                color: "#ffffff"
                                border.color: "#a9adb7"

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 8
                                    anchors.rightMargin: 8
                                    anchors.bottomMargin: 8
                                    spacing: 0
                                    Label {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 30
                                        text: "配置信息"
                                        font.pixelSize: 12
                                        font.bold: true
                                        color: "#4a4a4a"
                                        verticalAlignment: Text.AlignVCenter
                                        padding: 2
                                    }
                                    Rectangle {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 1
                                        color: "#e4e7ec"
                                    }
                                    Item { Layout.fillHeight: true }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 24
                            color: "#f6f3d7"
                            border.color: "#c7c19a"
                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 4
                                Label {
                                    text: "运行状态"
                                    color: "#4a4a4a"
                                    font.pixelSize: 12
                                }
                                Item { Layout.fillWidth: true }
                            }
                        }
                    }
                }

                Item {
                    id: settingsPage
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 0

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: 0
                            Layout.margins: 6

                            ColumnLayout {
                                Layout.preferredWidth: 290
                                Layout.maximumWidth: 290
                                Layout.fillHeight: true
                                spacing: 0

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 29
                                    color: "transparent"

                                    RowLayout {
                                        anchors.fill: parent
                                        spacing: 0
                                        Repeater {
                                            model: ["本地连接", "远程中转服务"]
                                            delegate: Rectangle {
                                                Layout.fillWidth: true
                                                Layout.preferredHeight: 28
                                                color: index === root.settingsConnectionTabIndex ? "#ffffff" : "#e7e7e7"
                                                border.color: "#bcbcbc"
                                                Text {
                                                    anchors.centerIn: parent
                                                    text: modelData
                                                    font.pixelSize: 12
                                                    font.bold: index === root.settingsConnectionTabIndex
                                                    color: "#333333"
                                                }
                                                MouseArea {
                                                    anchors.fill: parent
                                                    onClicked: {
                                                        if (root.settingsConnectionTabIndex === index) {
                                                            return
                                                        }
                                                        if (root.settingsConnectionTabIndex === 0) {
                                                            root.settingsLocalHost = settingHostField.text
                                                            root.settingsLocalPort = Number(settingPortField.text)
                                                        } else {
                                                            root.settingsRemoteHost = settingHostField.text
                                                            root.settingsRemotePort = Number(settingPortField.text)
                                                        }
                                                        root.settingsConnectionTabIndex = index
                                                        settingHostField.text = index === 0 ? root.settingsLocalHost : root.settingsRemoteHost
                                                        settingPortField.text = String(index === 0 ? root.settingsLocalPort : root.settingsRemotePort)
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 152
                                    color: "#ffffff"
                                    border.color: "#bcbcbc"
                                    clip: true

                                    ColumnLayout {
                                        anchors.fill: parent
                                        anchors.margins: 8
                                        spacing: 6

                                        RowLayout {
                                            Layout.fillWidth: true
                                            Label {
                                                text: root.settingsConnectionTabIndex === 0 ? "本地连接" : "远程中转服务"
                                                font.pixelSize: 12
                                                font.bold: true
                                                color: "#222222"
                                            }
                                            Item { Layout.fillWidth: true }
                                            Label {
                                                text: appState.connected
                                                    ? ("已连接  " + settingHostField.text + ":" + settingPortField.text)
                                                    : "未连接"
                                                font.pixelSize: 12
                                                font.bold: true
                                                color: appState.connected ? "#027a48" : "#6b7280"
                                                Layout.fillWidth: true
                                                elide: Text.ElideRight
                                                horizontalAlignment: Text.AlignRight
                                            }
                                        }

                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 4
                                            Label { text: "IP地址"; font.pixelSize: 12; color: "#222222"; Layout.preferredWidth: 78 }
                                            TextField {
                                                id: settingHostField
                                                Layout.fillWidth: true
                                                implicitHeight: 26
                                                font.pixelSize: 13
                                                topPadding: 3
                                                bottomPadding: 3
                                                text: root.settingsConnectionTabIndex === 0 ? root.settingsLocalHost : root.settingsRemoteHost
                                            }
                                        }

                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 4
                                            Label { text: "端口号"; font.pixelSize: 12; color: "#222222"; Layout.preferredWidth: 78 }
                                            TextField {
                                                id: settingPortField
                                                Layout.fillWidth: true
                                                implicitHeight: 26
                                                font.pixelSize: 13
                                                topPadding: 3
                                                bottomPadding: 3
                                                inputMethodHints: Qt.ImhDigitsOnly
                                                text: String(root.settingsConnectionTabIndex === 0 ? root.settingsLocalPort : root.settingsRemotePort)
                                            }
                                        }

                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 10
                                            Item { Layout.fillWidth: true }
                                            AppButton {
                                                text: "建立连接"
                                                implicitWidth: 92
                                                implicitHeight: 26
                                                onClicked: {
                                                    const port = Number(settingPortField.text)
                                                    if (root.settingsConnectionTabIndex === 0) {
                                                        root.settingsLocalHost = settingHostField.text
                                                        root.settingsLocalPort = port
                                                    } else {
                                                        root.settingsRemoteHost = settingHostField.text
                                                        root.settingsRemotePort = port
                                                    }
                                                    root.settingsRunStatus = "运行状态    正在建立连接"
                                                    appState.connectToDevice(settingHostField.text, port)
                                                }
                                            }
                                            AppButton {
                                                text: "断开连接"
                                                implicitWidth: 92
                                                implicitHeight: 26
                                                enabled: appState.connected
                                                onClicked: {
                                                    root.settingsRunStatus = "运行状态    未连接"
                                                    appState.disconnectFromDevice()
                                                }
                                            }
                                            Item { Layout.fillWidth: true }
                                        }
                                    }
                                }

                                Item { Layout.preferredHeight: 10 }

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 128
                                    color: "#ffffff"
                                    border.color: "#bcbcbc"
                                    clip: true

                                    ColumnLayout {
                                        anchors.fill: parent
                                        anchors.margins: 8
                                        spacing: 6
                                        Label { text: "存储"; font.pixelSize: 12; font.bold: true; color: "#222222" }
                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 4
                                            Label { text: "文件路径"; font.pixelSize: 12; color: "#222222"; Layout.preferredWidth: 78 }
                                            TextField {
                                                id: settingPathField
                                                Layout.fillWidth: true
                                                implicitHeight: 26
                                                font.pixelSize: 13
                                                topPadding: 3
                                                bottomPadding: 3
                                                text: "/tmp/deviceapp/device-app/frames"
                                            }
                                            AppButton { text: "..."; implicitWidth: 34; implicitHeight: 28 }
                                        }
                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 4
                                            Label { text: "文件名前缀"; font.pixelSize: 12; color: "#222222"; Layout.preferredWidth: 78 }
                                            TextField {
                                                id: settingPrefixField
                                                Layout.fillWidth: true
                                                implicitHeight: 26
                                                font.pixelSize: 13
                                                topPadding: 3
                                                bottomPadding: 3
                                                text: "Test"
                                            }
                                            AppButton {
                                                text: "设置"
                                                implicitWidth: 60
                                                implicitHeight: 28
                                                onClicked: root.settingsRunStatus = "运行状态    存储设置已保存"
                                            }
                                        }
                                        AppCheckBox { text: "自动保存"; checked: root.settingsAutoSave; onToggled: root.settingsAutoSave = checked }
                                    }
                                }
                                Item { Layout.fillHeight: true }
                            }

                            Rectangle {
                                Layout.preferredWidth: 1
                                Layout.fillHeight: true
                                color: "#bcbcbc"
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                color: "#ffffff"
                                border.color: "#bcbcbc"
                                clip: true
                                Layout.leftMargin: 10

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 6
                                    Label { text: "管道服务测试"; font.pixelSize: 12; font.bold: true; color: "#222222" }

                                    RowLayout {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 28
                                        spacing: 5
                                        TextField {
                                            id: payloadInput
                                            Layout.fillWidth: true
                                            implicitHeight: 26
                                            font.pixelSize: 13
                                            topPadding: 3
                                            bottomPadding: 3
                                            text: "&&$$:11:2,1;@@"
                                        }
                                        AppButton {
                                            text: "发送数据"
                                            implicitWidth: 76
                                            implicitHeight: 26
                                            onClicked: {
                                                if (showSendCheck.checked) {
                                                    serviceLog.text += "\\n[发送] " + payloadInput.text
                                                }
                                            }
                                        }
                                        AppCheckBox { id: showSendCheck; text: "显示发送"; checked: true }
                                        AppCheckBox { id: showReceiveCheck; text: "显示接收"; checked: true }
                                        AppButton { text: "清空数据"; implicitWidth: 76; implicitHeight: 26; onClicked: serviceLog.text = "" }
                                    }

                                    TextArea {
                                        id: serviceLog
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true
                                        readOnly: true
                                        Layout.topMargin: 2
                                        text: ""
                                        placeholderText: "暂无日志。发送管道数据后，日志会显示在这里。"
                                        background: Rectangle {
                                            color: "#ffffff"
                                            border.color: "#a9a9a9"
                                        }
                                    }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 24
                            color: "#f6f3d7"
                            border.color: "#c7c19a"
                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 4
                                anchors.rightMargin: 4
                                Label {
                                    text: root.settingsRunStatus
                                    color: "#222222"
                                    font.pixelSize: 12
                                }
                                Item { Layout.fillWidth: true }
                            }
                        }
                    }
                }
            }
        }
    }
}
