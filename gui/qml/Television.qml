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
            GradientStop { position: 0.43; color: "#081842" }
            GradientStop { position: 0.7; color: "#080d55" }
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
    Repeater {
        model: 2
        Item { required property int index; x: index === 0 ? 5 : tv.width - 175; y: 125; width: 170; height: 560
            Text { anchors.fill: parent; text: index === 0 ? "∫ f(x) dx\n\nH₂O  ⟶  O₂\n\nα + β = π\n\nF = ma\n\nλ = c / ν" : "Σ n²\n\nE = mc²\n\nΔt → ∞\n\nψ(x, t)\n\nπ ≈ 3.14159"; font.family: "DejaVu Sans"; font.pixelSize: 24; color: "#c7b768"; opacity: 0.18; rotation: index === 0 ? -9 : 9 }
            Rectangle { y: 185; width: 145; height: 145; radius: 72; color: "transparent"; border.color: "#5478a2"; opacity: 0.18; rotation: 35; Rectangle { anchors.centerIn: parent; width: 90; height: 90; color: "transparent"; border.color: "#b8b476" } }
        }
    }
}
