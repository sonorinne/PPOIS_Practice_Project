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
    Text { x: parent.width - 210; y: parent.height - 180; width: 185; height: 110; text: "π ≈ 3.14159\nλ = c / ν\n∞"; font.family: "DejaVu Sans"; color: "#c9b868"; font.pixelSize: 23; opacity: 0.17; lineHeight: 1.3; rotation: -6 }
    Repeater {
        model: 2
        Item { required property int index; x: index === 0 ? 5 : tv.width - 175; y: 125; width: 170; height: 560
            Text { anchors.fill: parent; text: index === 0 ? "∫ f(x) dx\n\nH₂O  ⟶  O₂\n\nα + β = π\n\nF = ma\n\nλ = c / ν" : "Σ n²\n\nE = mc²\n\nΔt → ∞\n\nψ(x, t)"; font.family: "DejaVu Sans"; font.pixelSize: 24; color: "#c7b768"; opacity: 0.18; rotation: index === 0 ? -9 : 9 }
            Canvas {
                y: 185; width: 155; height: 155; opacity: 0.2
                onPaint: {
                    let c=getContext("2d"); c.clearRect(0,0,width,height); c.lineWidth=1;
                    c.strokeStyle="#5478a2"; c.beginPath(); c.arc(77,77,72,0,Math.PI*2); c.stroke();
                    if(index === 0) {
                        c.strokeStyle="#b8b476"; c.save(); c.translate(77,77); c.rotate(Math.PI/4); c.strokeRect(-43,-43,86,86); c.restore();
                    } else {
                        // Orbital model replaces the second square-in-circle.
                        c.strokeStyle="#b8b476";
                        for(let angle=0;angle<3;angle++) { c.save(); c.translate(77,77); c.rotate(angle*Math.PI/3); c.scale(1,0.38); c.beginPath(); c.arc(0,0,58,0,Math.PI*2); c.stroke(); c.restore(); }
                        c.fillStyle="#c8c38a"; c.beginPath(); c.arc(77,77,5,0,Math.PI*2); c.fill();
                    }
                }
            }
        }
    }
}
