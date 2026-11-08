import QtQuick
Item {
    id: tv
    property bool effects: true
    Rectangle { anchors.fill: parent; color: "#060f29" }
    Rectangle {
        width: parent.width * 1.15; height: parent.height * 1.6
        anchors.centerIn: parent; rotation: -17
        gradient: Gradient {
            GradientStop { position: 0; color: "#081329" }
            GradientStop { position: 0.43; color: "#10266e" }
            GradientStop { position: 0.7; color: "#1918ad" }
            GradientStop { position: 1; color: "#070b20" }
        }
        opacity: 0.7
    }
    Repeater {
        model: 3
        Rectangle {
            required property int index
            width: 580 + index * 200; height: width; radius: width / 2
            x: tv.width - width * 0.75; y: tv.height - height * 0.6
            color: "transparent"; border.color: "#374a96"; border.width: 1; opacity: 0.16
        }
    }
    Text { x: parent.width - 250; y: 150; text: "Σ   φ\n     E = mc²\nπ      ∞"; color: "#c9b868"; font.pixelSize: 30; opacity: 0.12; lineHeight: 1.8; rotation: -10 }
    Canvas {
        anchors.fill: parent; visible: tv.effects; opacity: 0.13
        onPaint: {
            let c = getContext("2d"); c.clearRect(0, 0, width, height)
            c.fillStyle = "#000000"
            for (let y = 0; y < height; y += 4) c.fillRect(0, y, width, 1)
            for (let i = 0; i < 2200; i++) {
                c.fillStyle = Math.random() > 0.5 ? "#fff" : "#000"
                c.fillRect(Math.random() * width, Math.random() * height, 1, 1)
            }
        }
        Timer { interval: 130; repeat: true; running: tv.effects; onTriggered: parent.requestPaint() }
    }
    Rectangle {
        visible: tv.effects; width: parent.width; height: 25; opacity: 0.035; color: "#c6e5fa"
        SequentialAnimation on y {
            loops: Animation.Infinite; running: tv.effects
            NumberAnimation { from: -50; to: tv.height + 50; duration: 8200 }
            PauseAnimation { duration: 4500 }
        }
    }
}
