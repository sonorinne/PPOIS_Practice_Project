import QtQuick
import QtQuick.Controls
CheckBox {
    id: control
    hoverEnabled: true
    implicitHeight: 40
    implicitWidth: label.implicitWidth + 40
    spacing: 12
    indicator: Rectangle {
        x: control.leftPadding
        y: control.topPadding + (control.availableHeight - height) / 2
        width: 24; height: 24
        color: control.hovered ? "#fff0b0" : control.checked ? "#dfcc80" : "#132459"
        border.color: control.hovered ? "white" : "#847b5b"
        Text { anchors.centerIn: parent; text: control.checked ? "✓" : ""; font.family: "DejaVu Sans"; font.pixelSize: 20; color: "#0b1638" }
    }
    contentItem: Text {
        id: label
        text: control.text; font: control.font
        color: control.hovered ? "#fff0b0" : "#e2dcae"
        leftPadding: control.indicator.width + control.spacing
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}
