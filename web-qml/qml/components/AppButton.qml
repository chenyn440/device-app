import QtQuick 2.15
import QtQuick.Controls 2.15
import "../theme" as ThemeLib

Button {
    property bool selected: false
    implicitHeight: ThemeLib.Theme.size.buttonH
    implicitWidth: 54
    font.pixelSize: ThemeLib.Theme.font.body
    padding: 3
    background: Rectangle {
        color: (parent.selected || (parent.checkable && parent.checked))
               ? "#d7e8ff"
               : (parent.down ? "#dfe4ea" : (parent.hovered ? "#eef1f5" : "#f5f6f8"))
        border.color: (parent.selected || (parent.checkable && parent.checked)) ? "#7da2d8" : "#c7ccd3"
        radius: ThemeLib.Theme.size.radiusSm
    }
    contentItem: Text {
        text: parent.text
        font.pixelSize: ThemeLib.Theme.font.body
        color: (parent.selected || (parent.checkable && parent.checked)) ? "#1d4f91" : ThemeLib.Theme.color.textPrimary
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
