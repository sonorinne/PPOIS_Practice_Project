import QtQuick
Item {
    id: cat
    property bool animated: true
    property real bob: 0
    property real tail: 0
    property real eyelid: 0
    onTailChanged: drawing.requestPaint()
    onEyelidChanged: drawing.requestPaint()
    SequentialAnimation on tail { running: cat.animated; loops: Animation.Infinite; NumberAnimation { to: 0.1; duration: 1000 } NumberAnimation { to: -0.1; duration: 1000 } }
    SequentialAnimation on eyelid { running: cat.animated; loops: Animation.Infinite; PauseAnimation { duration: 3300 } NumberAnimation { to: 1; duration: 100 } NumberAnimation { to: 0; duration: 140 } }
    implicitWidth: 350; implicitHeight: 390
    SequentialAnimation on bob {
        running: cat.animated; loops: Animation.Infinite
        NumberAnimation { to: -5; duration: 1800; easing.type: Easing.InOutSine }
        NumberAnimation { to: 0; duration: 1800; easing.type: Easing.InOutSine }
    }
    Canvas {
        id: drawing
        anchors.fill: parent
        transform: Translate { y: cat.bob }
        onPaint: {
            let c = getContext("2d"); c.reset(); c.scale(width / 350, height / 390)
            function poly(p, color) {
                c.fillStyle = color; c.beginPath(); c.moveTo(p[0], p[1]);
                for (let i = 2; i < p.length; i += 2) c.lineTo(p[i], p[i+1]); c.closePath(); c.fill()
            }
            // Original low-poly scholar cat: deliberately angular, textured and slightly odd.
            c.fillStyle = "#030a1b"; c.beginPath(); c.ellipse(180,365,135,16,0,0,7); c.fill()
            c.save(); c.translate(239,320); c.rotate(cat.tail); c.translate(-239,-320);
            poly([239,281,292,270,316,296,309,339,279,359,228,357,222,338,275,336,286,310,270,300,242,314], "#587e94")
            c.restore();
            poly([102,239,217,233,248,346,212,365,93,359,78,335], "#395c79")
            poly([145,253,194,259,210,347,133,349], "#94ab9c")
            poly([95,263,129,278,112,347,80,350], "#699890")
            poly([207,267,231,284,247,350,214,350], "#203c68")
            poly([78,345,129,337,149,356,140,367,72,367], "#85b5a1")
            poly([197,344,235,341,259,361,250,368,189,365], "#78a293")
            poly([74,139,61,65,120,107,200,102,252,57,250,154,270,192,244,246,173,272,102,251,65,206], "#67968e")
            poly([61,65,107,114,76,144], "#3d466c")
            poly([252,57,243,146,210,108], "#354c78")
            poly([74,139,153,127,173,272,102,251,65,206], "#7aa599")
            poly([153,127,250,154,270,192,244,246,173,272], "#3c697d")
            poly([89,174,121,155,147,170,145,204,116,218,90,201], "#dad588")
            poly([173,170,204,153,230,172,234,204,203,217,178,204], "#c7d588")
            poly([112,168,124,164,126,202,114,205], "#0a1e30")
            poly([196,165,208,163,210,202,198,205], "#071a2c")
            if(cat.eyelid > 0) { c.fillStyle="#527e80"; c.fillRect(88,153,60,65*cat.eyelid); c.fillRect(173,153,63,65*cat.eyelid) }
            poly([147,214,175,212,162,231], "#bca5a2")
            c.strokeStyle = "#192e4a"; c.lineWidth = 3; c.beginPath(); c.moveTo(161,229); c.lineTo(163,246); c.lineTo(138,239); c.moveTo(163,246); c.lineTo(187,237); c.stroke()
            c.strokeStyle = "#bdd2ae"; c.lineWidth = 2
            for(let i=0;i<3;i++){ c.beginPath();c.moveTo(127,225+i*6);c.lineTo(42,211+i*15);c.moveTo(198,225+i*6);c.lineTo(284,210+i*15);c.stroke() }
            poly([96,91,211,83,224,112,170,130,99,119], "#253442")
            poly([42,79,162,36,282,69,164,106], "#bcc16b")
            poly([42,79,164,106,282,69,270,82,163,119,44,90], "#596854")
            c.strokeStyle = "#d9c56f"; c.lineWidth = 3; c.beginPath(); c.moveTo(267,78); c.lineTo(272,132); c.stroke()
            poly([267,130,277,130,281,150,263,149], "#c6b663")
            let seed = 71; for(let i=0;i<1800;i++) { seed=(seed*16807)%2147483647; let x=seed%230+50; seed=(seed*16807)%2147483647; let y=seed%300+45; c.fillStyle=i%2 ? Qt.rgba(1,1,1,0.025) : Qt.rgba(0,0,0,0.04); c.fillRect(x,y,2,2) }
        }
    }
}
