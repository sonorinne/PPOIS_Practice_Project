import QtQuick
import QtQuick.Controls
Slider {
    id: control
    property bool monochrome: false
    hoverEnabled: true
    implicitHeight: 36
    background: Rectangle {
        x: control.leftPadding; y: control.topPadding + control.availableHeight / 2 - 3
        width: control.availableWidth; height: 6; radius: 3; color: control.monochrome ? "#555555" : "#334264"
        Rectangle { width: control.visualPosition * parent.width; height: parent.height; radius: 3; color: control.monochrome ? "#dddddd" : "#d9c77d" }
    }
    handle: Rectangle {
        x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
        y: control.topPadding + (control.availableHeight - height) / 2
        width: 24; height: 24; radius: 12
        color: control.monochrome ? (!control.enabled ? "#555555" : control.pressed || hover.hovered ? "#ffffff" : "#aaaaaa") : !control.enabled ? "#536078" : control.pressed ? "#fff7d7" : hover.hovered ? "#fff0a0" : "#d4c17c"
        border.width: hover.hovered ? 2 : 1; border.color: control.monochrome ? "#ffffff" : hover.hovered ? "white" : "#83784e"
        HoverHandler { id: hover }
        Behavior on color { ColorAnimation { duration: 100 } }
    }
}
