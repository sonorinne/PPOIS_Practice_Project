import QtQuick
Item {
    id: noise
    property bool active: true
    visible: active
    Canvas {
        anchors.fill: parent; opacity: 0.15
        onPaint: {
            let c = getContext("2d"); c.clearRect(0,0,width,height)
            c.fillStyle = "#000000"
            for(let y=0;y<height;y+=4) c.fillRect(0,y,width,1)
            for(let i=0;i<2600;i++) {
                c.fillStyle = Math.random() > 0.5 ? "#ffffff" : "#000000"
                c.fillRect(Math.random()*width,Math.random()*height,1,1)
            }
            // Thin horizontal tracking streaks rather than blocky noise.
            c.fillStyle = Qt.rgba(0.85,0.9,1,0.3)
            for(let i=0;i<6;i++) c.fillRect(Math.random()*width,Math.random()*height,30+Math.random()*140,1)
        }
        Timer { interval: 100; repeat: true; running: noise.active; onTriggered: parent.requestPaint() }
    }
}
