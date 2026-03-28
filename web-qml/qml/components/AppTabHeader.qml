import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../theme" as ThemeLib

Item {
    id: root
    property var labels: []
    property int currentIndex: 0
    signal selected(int index)

    Rectangle {
        anchors.fill: parent
        color: "#eef1f5"
        border.color: ThemeLib.Theme.color.borderDefault
    }

    Row {
        anchors.verticalCenter: parent.verticalCenter
        spacing: 4
        x: 8

        Repeater {
            model: root.labels
            delegate: Button {
                required property int index
                required property string modelData
                text: modelData
                height: ThemeLib.Theme.size.tabH
                width: index === 0 ? 70 : 64
                font.pixelSize: ThemeLib.Theme.font.body
                checked: root.currentIndex === index
                checkable: true
                onClicked: {
                    root.currentIndex = index;
                    root.selected(index);
                }
                background: Rectangle {
                    color: parent.checked ? ThemeLib.Theme.color.primary : "#f2f3f5"
                    border.color: parent.checked ? ThemeLib.Theme.color.primary : "#cfd4dc"
                    radius: ThemeLib.Theme.size.radiusSm
                }
                contentItem: Text {
                    text: parent.text
                    color: parent.checked ? ThemeLib.Theme.color.textInverse : "#3f4754"
                    font.pixelSize: ThemeLib.Theme.font.body
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }
}
