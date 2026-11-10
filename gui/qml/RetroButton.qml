import QtQuick
import QtQuick.Controls
Button {
    id: control
    property bool accent: false
    property bool monochrome: false
    implicitHeight: 52
    implicitWidth: Math.max(140, label.implicitWidth + 40)
    Keys.onReturnPressed: if(enabled) clicked()
    Keys.onEnterPressed: if(enabled) clicked()
    hoverEnabled: true
    font.family: "Oswald"
    font.pixelSize: 26
    contentItem: Text {
        id: label
        text: control.text
        font: control.font
        color: control.monochrome ? (!control.enabled ? "#666666" : control.hovered || control.down ? "#000000" : "#ffffff") : !control.enabled ? "#65708b" : control.accent || control.hovered ? "#0b1638" : "#f2e4a2"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
    background: Rectangle {
        color: control.monochrome ? (!control.enabled ? "#222222" : control.down ? "#cccccc" : control.hovered ? "#ffffff" : "#171717") : !control.enabled ? "#15203a" : control.down ? "#fff0b5" : control.hovered ? "#fff0b0" : control.accent ? "#dfcc80" : "#132459"
        border.color: control.monochrome ? (control.hovered || control.activeFocus ? "white" : "#777777") : control.activeFocus || control.hovered ? "white" : "#847b5b"
        border.width: control.activeFocus ? 3 : 2
        Behavior on color { ColorAnimation { duration: 100 } }
        Rectangle { x: 3; y: 3; width: parent.width - 6; height: 1; color: "#ffffff"; opacity: 0.15 }
    }
}
