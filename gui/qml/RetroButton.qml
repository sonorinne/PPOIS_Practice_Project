import QtQuick
import QtQuick.Controls
Button {
    id: control
    property bool accent: false
    implicitHeight: 52
    implicitWidth: Math.max(140, label.implicitWidth + 40)
    hoverEnabled: true
    font.family: "Tiny5"
    font.pixelSize: 26
    contentItem: Text {
        id: label
        text: control.text
        font: control.font
        color: !control.enabled ? "#65708b" : control.accent || control.hovered ? "#0b1638" : "#f2e4a2"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
    background: Rectangle {
        color: !control.enabled ? "#15203a" : control.down ? "#fff0b5" : control.accent || control.hovered ? "#dfcc80" : "#132459"
        border.color: control.activeFocus ? "white" : "#847b5b"
        border.width: control.activeFocus ? 3 : 1
        Rectangle { x: 3; y: 3; width: parent.width - 6; height: 1; color: "#ffffff"; opacity: 0.15 }
    }
}
