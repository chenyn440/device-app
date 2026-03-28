pragma Singleton
import QtQuick 2.15
import "Tokens.js" as Tokens

QtObject {
    readonly property var color: Tokens.color
    readonly property var font: Tokens.font
    readonly property var size: Tokens.size
    readonly property var gap: Tokens.gap
}
