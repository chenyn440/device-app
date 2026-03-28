import QtQuick 2.15
import QtQuick.Controls 2.15
import "../theme" as ThemeLib

CheckBox {
    property bool compact: text.length === 0

    font.pixelSize: ThemeLib.Theme.font.body
    spacing: compact ? 0 : 4
    implicitHeight: compact ? 14 : 18
    leftPadding: compact ? indicator.width : indicator.width + 6

    indicator: Rectangle {
        x: 0
        y: parent.topPadding + (parent.availableHeight - height) / 2
        width: ThemeLib.Theme.size.checkboxIndicator
        height: ThemeLib.Theme.size.checkboxIndicator
        radius: 1
        border.color: "#b8bec8"
        color: parent.checked ? "#f9fbff" : "#f3f4f6"

        Text {
            anchors.centerIn: parent
            text: parent.parent.checked ? "✓" : ""
            font.pixelSize: 10
            color: ThemeLib.Theme.color.textPrimary
        }
    }

    contentItem: Text {
        leftPadding: parent.compact ? 0 : parent.spacing
        text: parent.text
        font.pixelSize: ThemeLib.Theme.font.body
        color: ThemeLib.Theme.color.textPrimary
        verticalAlignment: Text.AlignVCenter
    }
}
