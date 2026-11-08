import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

ApplicationWindow {
    id: window
    width: 1280; height: 800; minimumWidth: 960; minimumHeight: 640
    visible: true; title: "Своя игра · интеллектуальный тренажёр"
    color: "#040a1b"
    property var s: game.state
    property string page: "menu"
    property int selectedCell: -1
    property bool flashOn: true
    property string roundLabel: ""
    function beginSelection(cell) { selectedCell = cell; rowFlash.restart() }
    function playCue(cue) { stopCues(); if(window.s.sounds) cue.play() }
    function stopCues() { rightSound.stop(); wrongSound.stop(); catSound.stop(); fanfareSound.stop(); endSound.stop(); noAnswerSound.stop() }
    property int serial: 0
    property var chosenCategories: []
    property string playerName: s.player
    property string backendScreen: "menu"
    property bool quitting: false
    property bool exportingIcon: false
    onClosing: function(close) { if (!quitting && ["board","question","cat","roundEnd"].indexOf(page) >= 0) { close.accepted = false; exitDialog.open() } }
    Shortcut { sequence: "Return"; enabled: page === "question" && window.s.answered; onActivated: game.advance() }
    font.family: "Oswald"
    font.pixelSize: 24
    Connections {
        target: game
        function onQuestionOpened() { if(window.s.sounds && !catSound.playing) opening.play() }
        function onRoundStarted(number) { window.roundLabel = "РАУНД " + number; roundIntro.restart(); window.playCue(fanfareSound) }
        function onTrainingStarted() { window.playCue(fanfareSound) }
        function onModeFinished() { window.playCue(endSound) }
        function onRoundFinished() { window.roundLabel = "РАУНД ЗАВЕРШЁН"; roundIntro.stop(); roundBanner.visible = true; titleEntrance.restart(); window.playCue(endSound) }
        function onCatOpened() { window.playCue(catSound) }
        function onAnswerRevealed() { window.playCue(noAnswerSound) }
        function onResponseSubmitted(correct) { window.playCue(correct ? rightSound : wrongSound) }
        function onChanged() { if (game.state.serial !== window.serial) { window.serial = game.state.serial; answerInput.text = ""; reveal.restart(); Qt.callLater(function() { answerInput.forceActiveFocus() }) } if (game.state.screen !== window.backendScreen) { window.backendScreen = game.state.screen; window.page = game.state.screen } }
    }
    onPageChanged: { if(page !== "board" && page !== "roundEnd") { roundIntro.stop(); roundBanner.visible = false } reveal.restart(); answerInput.text = ""; selectedCell = -1;  audio.stop(); if(page === "question") Qt.callLater(function() { answerInput.forceActiveFocus() }) }
    Item {
        id: stage
        width: 1280; height: 800
        anchors.centerIn: parent
        scale: Math.min(window.width / width, window.height / height)
        clip: true
        Television { anchors.fill: parent; effects: window.s.effects }
        Text { x: 56; y: 27; text: "СВОЯ ИГРА / ИНТЕЛЛЕКТУАЛЬНЫЙ ТРЕНАЖЁР"; color: "#b2c6df"; font.pixelSize: 21; font.letterSpacing: 1.1 }
        Row {
            anchors.right: parent.right; anchors.rightMargin: 44; y: 22; spacing: 16
            RetroButton { objectName: "settingsButton"; text: "НАСТРОЙКИ"; implicitHeight: 38; font.pixelSize: 18; onClicked: settingsDialog.open() }
            RetroButton { text: "МЕНЮ"; implicitHeight: 38; implicitWidth: 100; font.pixelSize: 20; visible: page !== "menu"; onClicked: { if (["board", "question", "cat", "roundEnd"].indexOf(page) >= 0) leaveDialog.open(); else { game.menu(); page = "menu" } } }
            RetroButton { objectName: "exitButton"; text: "ВЫЙТИ"; implicitHeight: 38; implicitWidth: 100; font.pixelSize: 20; onClicked: { if (["board","question","cat","roundEnd"].indexOf(page) >= 0) exitDialog.open(); else { window.quitting = true; window.close() } } }
        }
        Rectangle { x: 44; y: 72; width: 1192; height: 1; color: "#4a5b85" }

        Item {
            id: content
            x: 56; y: 100; width: 1168; height: 590
            ParallelAnimation { id: reveal; NumberAnimation { target: content; property: "opacity"; from: 0; to: 1; duration: window.s.effects ? 240 : 0 } NumberAnimation { target: content; property: "scale"; from: 0.975; to: 1; duration: window.s.effects ? 300 : 0; easing.type: Easing.OutCubic } }
            Item {
                anchors.fill: parent; visible: page === "menu"
                Text { y: 58; text: "ДОБРО ПОЖАЛОВАТЬ В ЭФИР!"; color: "#e1ce85"; font.pixelSize: 22; font.letterSpacing: 2 }
                Text { objectName: "menuTitle"; y: 90; width: 550; height: 130; text: "СВОЯ ИГРА"; font.bold: true; font.pixelSize: 88; font.letterSpacing: 1; color: "#edf4df"; style: Text.Raised; styleColor: "#446ca1"; fontSizeMode: Text.Fit; minimumPixelSize: 65 }
                TextField { id: nameInput; x: 0; y: 265; width: 500; height: 49; text: window.playerName; placeholderText: "Как тебя представить?"; maximumLength: 24; color: "#f2edcf"; font.pixelSize: 25; onTextEdited: window.playerName = text; background: Rectangle { color: "#081635"; border.color: "#566c99" } }
                Column { y: 350; spacing: 18
                    RetroButton { objectName: "playButton"; text: "ИГРАТЬ"; accent: true; implicitWidth: 500; onClicked: game.startClassic(nameInput.text) }
                    RetroButton { objectName: "trainingButton"; text: "ТРЕНИРОВКА"; implicitWidth: 500; onClicked: page = "setup" }
                    RetroButton { text: "АРХИВ РЕЗУЛЬТАТОВ"; implicitWidth: 500; onClicked: game.showHistory() }
                }
                Cat { objectName: "menuCat"; x: 600; y: -20; width: 600; height: 690; animated: true }
            }
            Item {
                anchors.fill: parent; visible: page === "board"
                Text { anchors.horizontalCenter: parent.horizontalCenter; y: 2; text: "РАУНД " + window.s.round; color: "#e4d590"; font.pixelSize: 48; font.bold: true }
                Column {
                    y: 96; width: parent.width; spacing: 0
                    Repeater {
                        model: window.s.rows
                        delegate: Row {
                            id: boardRow
                            required property var modelData
                            required property int index
                            property int rowNumber: index
                            width: 1168; height: 73
                            Rectangle {
                                width: 348; height: 73
                                color: (window.flashOn && window.selectedCell >= 0 && Math.floor(window.selectedCell / 5) === boardRow.rowNumber) ? "#646039" : "#142b71"
                                border.color: "#566caa"
                                Text { anchors.fill: parent; anchors.margins: 14; text: boardRow.modelData.name.toUpperCase(); color: "#e1d597"; font.pixelSize: 25; wrapMode: Text.WordWrap; verticalAlignment: Text.AlignVCenter; lineHeight: 0.9 }
                            }
                            Repeater {
                                model: boardRow.modelData.cells
                                delegate: Rectangle {
                                    required property var modelData
                                    width: 164; height: 73
                                    color: window.flashOn && window.selectedCell >= 0 && Math.floor(window.selectedCell / 5) === boardRow.rowNumber ? "#646039" : cellMouse.containsMouse && !modelData.used ? "#244ba1" : "#132c78"
                                    border.color: activeFocus ? "#fff6d0" : "#566caa"
                                    border.width: activeFocus ? 3 : 1
                                    activeFocusOnTab: !modelData.used
                                    Accessible.role: Accessible.Button
                                    Accessible.name: boardRow.modelData.name + " " + modelData.cost
                                    Keys.onReturnPressed: if (!modelData.used && window.selectedCell < 0) { window.beginSelection(modelData.index) }
                                    Keys.onSpacePressed: if (!modelData.used && window.selectedCell < 0) { window.beginSelection(modelData.index) }
                                    Text { anchors.centerIn: parent; text: parent.modelData.used ? "" : parent.modelData.cost; font.pixelSize: 42; color: "#e8dea8" }
                                    MouseArea {
                                        id: cellMouse; anchors.fill: parent; hoverEnabled: true
                                        enabled: !parent.modelData.used && window.selectedCell < 0
                                        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor


                                        onClicked: { window.beginSelection(parent.modelData.index) }
                                    }
                                }
                            }
                        }
                    }
                }
                Row { y: 522; spacing: 16
                }
            }
            Item {
                anchors.fill: parent; visible: page === "cat"
                Cat { x: 82; y: 80; width: 350; height: 390; animated: true }
                Column { x: 510; y: 110; spacing: 26; width: 640
                    Text { text: "КОТ В МЕШКЕ"; color: "#f0df99"; font.pixelSize: 64 }
                    Text { width: 630; wrapMode: Text.WordWrap; text: "Тема: " + window.s.catTheme + "\nСтоимость выбираешь ты."; color: "#c2cee2"; font.pixelSize: 30; lineHeight: 1.1 }
                    Row { spacing: 20; RetroButton { text: "100"; implicitWidth: 200; onClicked: game.catStake(100) } RetroButton { text: "500"; implicitWidth: 200; onClicked: game.catStake(500) } }
                }
            }
            Item {
                anchors.fill: parent; visible: page === "question"
                Text { text: (window.s.theme || "").toUpperCase() + " / " + (window.s.cost || "") + ""; color: "#dece85"; font.pixelSize: 25 }
                Item {
                    id: mediaArea; x: window.s.kind === "image" ? 75 : 300; y: 42; width: window.s.kind === "image" ? 1018 : 568; height: window.s.kind === "image" ? (window.s.answered ? 260 : 380) : 218
                    visible: window.s.kind !== "text"
                    Image { anchors.fill: parent; visible: window.s.kind === "image"; source: window.s.kind === "image" ? window.s.media : ""; fillMode: Image.PreserveAspectFit; smooth: true }
                    Text { anchors.centerIn: parent; visible: window.s.kind === "image" && parent.children[0].status === Image.Error; text: "Изображение недоступно"; color: "#e7bb97"; font.pixelSize: 24 }
                    Canvas {
                        id: clef; anchors.centerIn: parent; width: 130; height: 202; visible: window.s.kind === "audio"
                        transform: Rotation { id: spin; origin.x: 65; origin.y: 101; axis.x: 0; axis.y: 1; axis.z: 0 }
                        NumberAnimation { target: spin; property: "angle"; from: 0; to: 360; duration: 5600; loops: Animation.Infinite; running: clef.visible && page === "question" && window.s.effects && !window.s.answered }
                        onPaint: {
                            let c = getContext("2d"); c.clearRect(0,0,width,height)
                            let g=c.createLinearGradient(20,0,110,0); g.addColorStop(0,"#577eaa");g.addColorStop(0.4,"#f7f4c9");g.addColorStop(0.65,"#bacae0");g.addColorStop(1,"#d1bd68")
                            c.strokeStyle=g;c.lineWidth=9;c.lineCap="round";c.beginPath()
                            c.moveTo(65,187);c.bezierCurveTo(94,209,95,172,79,124);c.bezierCurveTo(66,87,35,40,72,8);c.bezierCurveTo(110,59,24,88,23,123);c.bezierCurveTo(16,174,111,167,105,121);c.bezierCurveTo(99,86,43,99,51,131);c.bezierCurveTo(55,145,72,145,77,135);c.stroke()
                        }
                    }
                }
                Text {
                    x: 115; y: window.s.kind === "image" ? (window.s.answered ? 309 : 430) : window.s.kind === "text" ? 88 : 272; width: 938; height: window.s.kind === "image" ? 65 : window.s.kind === "text" ? 256 : 106
                    text: window.s.text || ""; color: "#f0e6b8"; font.family: "Oswald"; font.pixelSize: window.s.kind === "text" ? 36 : 28
                    horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; wrapMode: Text.WordWrap; lineHeight: 0.96
                    fontSizeMode: Text.Fit; minimumPixelSize: 22
                }
                Row { x: 425; y: 236; spacing: 10; visible: window.s.kind === "audio" && !window.s.answered
                    RetroButton { text: audio.playbackState === MediaPlayer.PlayingState ? "ПАУЗА" : "СЛУШАТЬ"; implicitHeight: 32; font.pixelSize: 18; onClicked: audio.playbackState === MediaPlayer.PlayingState ? audio.pause() : audio.play() }
                    RetroButton { text: "СНАЧАЛА"; implicitHeight: 32; font.pixelSize: 18; onClicked: { audio.position = 0; audio.play() } }
                }
                Text { x: 115; y: 371; visible: window.s.kind === "audio" && audio.error !== MediaPlayer.NoError; text: "Не удалось воспроизвести аудио. Проверьте файл и устройство звука."; color: "#e9b398"; font.pixelSize: 20 }
                Row { objectName: "answerRow"; anchors.horizontalCenter: parent.horizontalCenter; y: window.s.kind === "image" ? 500 : 407; spacing: 12; visible: !window.s.answered
                    TextField { id: answerInput; objectName: "answerInput"; width: 635; height: 53; placeholderText: "Твой ответ…"; placeholderTextColor: "#a2b5cf"; color: "#eff0db"; font.family: "DejaVu Sans"; font.pixelSize: 22; selectByMouse: true; maximumLength: 250
                        background: Rectangle { color: "#081737"; border.color: answerInput.activeFocus ? "#e5d58b" : "#6c84b2" }
                        onAccepted: if (page === "question") { game.submit(text); audio.stop() }
                    }
                    RetroButton { objectName: "submitButton"; text: "ОТВЕТИТЬ"; accent: true; onClicked: { game.submit(answerInput.text); audio.stop() } }
                }
                RetroButton { anchors.horizontalCenter: parent.horizontalCenter; y: window.s.kind === "image" ? 560 : 482; implicitHeight: window.s.kind === "image" ? 36 : 52; visible: !window.s.answered; text: "ПОКАЗАТЬ ОТВЕТ"; font.pixelSize: 21; onClicked: { game.revealAnswer(); audio.stop() } }
                Column { x: 100; y: 401; width: 968; spacing: 6; visible: window.s.answered
                    Text { width: parent.width; horizontalAlignment: Text.AlignHCenter; text: window.s.correct ? "ВЕРНО!" : "ОТВЕТ: " + (window.s.primaryAnswer || ""); color: window.s.correct ? "#b7dca0" : "#f0d394"; font.pixelSize: 31; wrapMode: Text.WordWrap }
                    Text { width: parent.width; horizontalAlignment: Text.AlignHCenter; text: "Твой ответ: " + (window.s.answer || "пропуск"); color: "#a5b8d5"; font.family: "DejaVu Sans"; font.pixelSize: 17; elide: Text.ElideRight }
                    Item { width: 1; height: 24 }
                    RetroButton { anchors.horizontalCenter: parent.horizontalCenter; text: "ПРОДОЛЖИТЬ"; accent: true; onClicked: game.advance() }
                }
            }
            Item {
                anchors.fill: parent; visible: page === "setup"
                Text { text: "ТРЕНИРОВКА"; color: "#eee2a6"; font.pixelSize: 64; font.bold: true }
                RetroCheckBox { id: randomThemes; objectName: "randomThemes"; y: 108; text: "Случайные темы"; checked: window.chosenCategories.length === 0; palette.windowText: "#e2dcae"; font.pixelSize: 24; onClicked: { window.chosenCategories = [] } }
                ScrollView { y: 160; width: 590; height: 390; clip: true; contentWidth: availableWidth
                    Column { width: 565; spacing: 5
                        Repeater { model: game.categories
                            RetroCheckBox { required property var modelData; width: 560; height: 44; text: modelData.name; font.pixelSize: 23; palette.windowText: "#e2dcae"; checked: window.chosenCategories.indexOf(modelData.id) >= 0
                                onClicked: { let ids = window.chosenCategories.slice(); let i = ids.indexOf(modelData.id); if (checked && i < 0) ids.push(modelData.id); else if (!checked && i >= 0) ids.splice(i,1); window.chosenCategories = ids }
                                contentItem: Text { text: parent.text; font: parent.font; color: "#e2dcae"; leftPadding: parent.indicator.width + parent.spacing; verticalAlignment: Text.AlignVCenter; elide: Text.ElideRight }
                            }
                        }
                    }
                }
                Column { x: 660; y: 130; width: 440; spacing: 18
                    Text { text: "СЛОЖНОСТЬ И ДЛИНА ТРЕНИРОВКИ"; color: "#e1ce85"; font.pixelSize: 24 }
                    Row { spacing: 24
                        Column { spacing: 8; Text { text: "ОТ СТОИМОСТИ"; color: "#a4b9d4"; font.pixelSize: 20 } ComboBox { id: minCost; model: [100,200,300,400,500]; width: 190; font.pixelSize: 24 } }
                        Column { spacing: 8; Text { text: "ДО СТОИМОСТИ"; color: "#a4b9d4"; font.pixelSize: 20 } ComboBox { id: maxCost; model: [100,200,300,400,500]; currentIndex: 4; width: 190; font.pixelSize: 24 } }
                    }
                    Text { text: "КОЛИЧЕСТВО ВОПРОСОВ"; color: "#a4b9d4"; font.pixelSize: 20 }
                    ComboBox { id: questionCount; model: [5,10,15,20,30]; currentIndex: 1; width: 404; font.pixelSize: 24 }
                    Text { width: 420; text: "Без штрафа за ошибки.\nС повторением трудных вопросов."; color: "#97aece"; font.pixelSize: 23; wrapMode: Text.WordWrap }
                    RetroButton { objectName: "startTrainingButton"; text: "НАЧАТЬ ТРЕНИРОВКУ"; implicitWidth: 404; accent: true; enabled: minCost.currentIndex <= maxCost.currentIndex; onClicked: game.startTraining(window.playerName, window.chosenCategories, Number(minCost.currentText), Number(maxCost.currentText), Number(questionCount.currentText)) }
                }
            }
            Item {
                anchors.fill: parent; visible: page === "results"
                Text { text: "ЭФИР ЗАВЕРШЁН"; color: "#efe1a2"; font.pixelSize: 55 }
                Text { y: 80; text: window.s.score + " БАЛЛОВ    /    " + window.s.correctCount + " ИЗ " + window.s.answeredCount + " ВЕРНО"; color: "#b8cee5"; font.pixelSize: 34 }
                Text { y: 140; text: window.s.mistakes.length ? "ВОПРОСЫ ДЛЯ ПОВТОРЕНИЯ" : "Отличная работа. Увидимся в следующем эфире."; color: "#d3c584"; font.pixelSize: 24 }
                ScrollView { x: 0; y: 185; width: parent.width; height: 310; clip: true
                    Column { width: 1120; spacing: 18
                        Repeater { model: window.s.mistakes
                            Column { required property var modelData; width: 1090; spacing: 5
                                Text { width: parent.width; text: modelData.theme.toUpperCase() + " / " + modelData.text; color: "#b4c6df"; font.family: "DejaVu Sans"; font.pixelSize: 18; wrapMode: Text.WordWrap }
                                Text { width: parent.width; text: "Ответ: " + modelData.answer; color: "#e5d995"; font.pixelSize: 25; wrapMode: Text.WordWrap }
                            }
                        }
                    }
                }
                Row { y: 520; spacing: 16
                    RetroButton { text: "В МЕНЮ"; accent: true; onClicked: game.menu() }
                    RetroButton { text: "ПОВТОРИТЬ ОШИБКИ"; visible: window.s.training && window.s.mistakes.length > 0; onClicked: game.reviewMistakes() }
                    RetroButton { text: "АРХИВ"; onClicked: game.showHistory() }
                }
            }
            Item {
                anchors.fill: parent; visible: page === "history"
                Text { text: "АРХИВ ЭФИРОВ"; color: "#eee0a2"; font.pixelSize: 56 }
                Text { y: 80; text: window.s.history.length ? "Последние 50 сессий сохраняются на этом компьютере." : "Пока пусто. Начни с табло или тренировки."; color: "#a3b9d8"; font.pixelSize: 24 }
                ScrollView { y: 140; width: parent.width; height: 415; clip: true
                    Column { width: 1140; spacing: 12
                        Repeater { model: window.s.history
                            Rectangle { required property var modelData; width: 1120; height: 78; color: "#112858"; border.color: "#425b8b"
                                Text { x: 18; y: 10; text: parent.modelData.player + " / " + parent.modelData.mode + " / " + parent.modelData.date; color: "#a6bbd7"; font.pixelSize: 22 }
                                Text { x: 18; y: 40; text: parent.modelData.score + " баллов · " + parent.modelData.correct + " из " + parent.modelData.total + " верно"; color: "#e4d79a"; font.pixelSize: 26 }
                            }
                        }
                    }
                }
            }
        }
        Rectangle {
            id: roundBanner; objectName: "roundBanner"; x: 44; y: 88; width: 1192; height: 630; visible: false; color: "#081435"; z: 30
            border.color: "#827744"; border.width: 1
            Text { id: roundTitle; anchors.centerIn: parent; transform: Translate { id: titleShift } text: window.roundLabel; color: "#f4e4a0"; font.pixelSize: 90; font.bold: true }
            MouseArea { anchors.fill: parent }
            ParallelAnimation { id: titleEntrance; NumberAnimation { target: titleShift; property: "y"; from: -100; to: 0; duration: 420; easing.type: Easing.OutBack } NumberAnimation { target: roundTitle; property: "opacity"; from: 0; to: 1; duration: 260 } }
            SequentialAnimation { id: roundIntro; PropertyAction { target: roundBanner; property: "visible"; value: true } ScriptAction { script: titleEntrance.restart() } PauseAnimation { duration: 2200 } PropertyAction { target: roundBanner; property: "visible"; value: false } }
        }
        BroadcastNoise { anchors.fill: parent; active: window.s.effects; z: 40 }
        IconArtwork { objectName: "exportIcon"; width: 512; height: 512; visible: window.exportingIcon; z: 100 }
        Text { x: 56; y: 698; width: 1168; text: window.s.error; color: "#f3b2a1"; font.pixelSize: 20; elide: Text.ElideRight }
        Rectangle { x: 44; y: 730; width: 1192; height: 1; color: "#4a5b85" }
        Row { x: 56; y: 751; spacing: 40
            Text { text: window.s.player.toUpperCase(); color: "#e7d791"; font.pixelSize: 25 }
            Text { text: "СЧЁТ  " + window.s.score; color: "#edf1de"; font.pixelSize: 25 }
            Text { text: "ВЕРНО  " + window.s.correctCount + "   /   ОШИБКИ  " + window.s.wrongCount; color: "#9cb5d5"; font.pixelSize: 23 }
        }
        Text { anchors.right: parent.right; anchors.rightMargin: 56; y: 754; text: page === "question" && window.s.training ? "ВОПРОС " + window.s.number + " / " + window.s.total : "ЛИЧНЫЙ ЭФИР · 01"; color: "#839dc3"; font.pixelSize: 21 }
    }
    ShaderEffectSource {
        id: screenTexture
        sourceItem: stage
        hideSource: curvedScreen.visible
        live: curvedScreen.visible
        visible: false
        textureSize: Qt.size(1280,800)
    }
    ShaderEffect {
        id: curvedScreen; objectName: "curvedScreen"
        anchors.centerIn: parent
        width: stage.width; height: stage.height; scale: stage.scale
        visible: window.s.curvature && GraphicsInfo.api !== GraphicsInfo.Software
        property variant source: screenTexture
        fragmentShader: "qrc:/gui/shaders/curved.frag.qsb"
    }
    SequentialAnimation {
        id: rowFlash
        PropertyAction { target: window; property: "flashOn"; value: true }
        PauseAnimation { duration: window.s.effects ? 150 : 0 }
        PropertyAction { target: window; property: "flashOn"; value: false }
        PauseAnimation { duration: window.s.effects ? 120 : 0 }
        PropertyAction { target: window; property: "flashOn"; value: true }
        ScriptAction { script: selectionTimer.restart() }
    }
    Timer { id: selectionTimer; interval: window.s.effects ? 130 : 1; onTriggered: { if(page === "board") game.select(window.selectedCell) } }
    AudioOutput { id: audioOutput; volume: 0.7 }
    MediaPlayer { id: audio; audioOutput: audioOutput; source: page === "question" && window.s.kind === "audio" ? window.s.media : ""; onSourceChanged: { if(source.toString().length > 0) play() } }
    AudioOutput { id: musicOutput; volume: window.s.music && !(page === "question" && window.s.kind === "audio" && !window.s.answered) ? window.s.musicVolume * 0.3 : 0 }
    MediaPlayer { id: backgroundMusic; source: "qrc:/gui/assets/background.mp3"; audioOutput: musicOutput; loops: MediaPlayer.Infinite; Component.onCompleted: { if(window.s.music) play() } }
    Connections { target: game; function onChanged() { if(!window.s.sounds) { window.stopCues(); opening.stop(); previewSound.stop() } if(window.s.music && backgroundMusic.playbackState !== MediaPlayer.PlayingState) backgroundMusic.play(); else if(!window.s.music && backgroundMusic.playbackState === MediaPlayer.PlayingState) backgroundMusic.pause() } }
    SoundEffect { id: catSound; source: "qrc:/gui/assets/cat.wav"; volume: window.s.soundVolume }
    SoundEffect { id: fanfareSound; source: "qrc:/gui/assets/fanfare.wav"; volume: window.s.soundVolume }
    SoundEffect { id: endSound; source: "qrc:/gui/assets/roundEnd.wav"; volume: window.s.soundVolume }
    SoundEffect { id: noAnswerSound; source: "qrc:/gui/assets/noanswer.wav"; volume: window.s.soundVolume }
    SoundEffect { id: previewSound; source: "qrc:/gui/assets/meow.wav"; volume: window.s.soundVolume }
    Timer { id: previewTimer; interval: 150; onTriggered: { if(window.s.sounds && !previewSound.playing) previewSound.play() } }
    SoundEffect { id: opening; source: "qrc:/gui/assets/open.wav"; volume: window.s.soundVolume }
    SoundEffect { id: rightSound; source: "qrc:/gui/assets/right.wav"; volume: window.s.soundVolume }
    SoundEffect { id: wrongSound; source: "qrc:/gui/assets/wrong.wav"; volume: window.s.soundVolume }
    Dialog {
        id: settingsDialog; objectName: "settingsDialog"; anchors.centerIn: parent; width: 510; modal: true; title: "Настройки эфира"; standardButtons: Dialog.Close
        palette.window: "#101010"; palette.windowText: "#ffffff"; palette.text: "#ffffff"; palette.buttonText: "#ffffff"; palette.button: "#222222"; palette.highlight: "#ffffff"; palette.highlightedText: "#000000"
        background: Rectangle { color: "#101010"; border.color: "#888888" }
        header: Label { text: settingsDialog.title; color: "#ffffff"; font.bold: true; padding: 16; font.pixelSize: 26 }
        footer: DialogButtonBox { standardButtons: settingsDialog.standardButtons; spacing: 20; padding: 12; background: Rectangle { color: "transparent" } delegate: RetroButton { monochrome: true; implicitWidth: 110; implicitHeight: 44 } }
        Column { width: 450; spacing: 12
            RetroCheckBox { monochrome: true; text: "Фоновая музыка"; checked: window.s.music; onClicked: game.setAudio(checked, window.s.sounds, window.s.musicVolume, window.s.soundVolume) }
            RetroSlider { monochrome: true; width: 430; from: 0; to: 1; value: window.s.musicVolume; onMoved: game.setAudio(window.s.music,window.s.sounds,value,window.s.soundVolume) }
            RetroCheckBox { monochrome: true; text: "Звуки вопросов и ответов"; checked: window.s.sounds; onClicked: game.setAudio(window.s.music,checked,window.s.musicVolume,window.s.soundVolume) }
            RetroSlider { monochrome: true; width: 430; from: 0; to: 1; value: window.s.soundVolume; enabled: window.s.sounds; onMoved: { game.setAudio(window.s.music,window.s.sounds,window.s.musicVolume,value); if(!previewTimer.running && !previewSound.playing) previewTimer.start() } }
            RetroCheckBox { monochrome: true; text: "Выпуклый экран (CRT)"; checked: window.s.curvature; onClicked: game.setCurvature(checked) }
            RetroCheckBox { monochrome: true; text: "Помехи старого экрана"; checked: window.s.effects; onClicked: game.setEffects(checked) }
        }
    }
    Timer { id: exitAfterCue; interval: window.s.sounds ? 2800 : 1; onTriggered: window.close() }
    Dialog {
        id: exitDialog; anchors.centerIn: parent; width: 510; modal: true; title: "Выйти из приложения?"; standardButtons: Dialog.Yes | Dialog.No
        palette.window: "#101010"; palette.windowText: "#ffffff"; palette.text: "#ffffff"; palette.buttonText: "#ffffff"; palette.button: "#222222"; palette.highlight: "#ffffff"; palette.highlightedText: "#000000"
        background: Rectangle { color: "#101010"; border.color: "#888888" }
        header: Label { text: exitDialog.title; color: "#ffffff"; font.bold: true; padding: 16; font.pixelSize: 26 }
        footer: DialogButtonBox { standardButtons: exitDialog.standardButtons; spacing: 20; padding: 12; background: Rectangle { color: "transparent" } delegate: RetroButton { monochrome: true; implicitWidth: 110; implicitHeight: 44 } }
        Label { text: "Отвеченные вопросы сохранятся в архиве."; font.pixelSize: 22 }
        onAccepted: { selectionTimer.stop(); game.finish(); window.quitting = true; exitAfterCue.start() }
    }
    Dialog {
        id: leaveDialog; anchors.centerIn: parent; width: 520; modal: true; title: "Завершить сессию?"; standardButtons: Dialog.Ok | Dialog.Cancel
        palette.window: "#101010"; palette.windowText: "#ffffff"; palette.text: "#ffffff"; palette.buttonText: "#ffffff"; palette.button: "#222222"; palette.highlight: "#ffffff"; palette.highlightedText: "#000000"
        background: Rectangle { color: "#101010"; border.color: "#888888" }
        header: Label { text: leaveDialog.title; color: "#ffffff"; font.bold: true; padding: 16; font.pixelSize: 26 }
        footer: DialogButtonBox { standardButtons: leaveDialog.standardButtons; spacing: 20; padding: 12; background: Rectangle { color: "transparent" } delegate: RetroButton { monochrome: true; implicitWidth: 110; implicitHeight: 44 } }
        Label { text: "Результаты ответов сохранятся в архиве.\nНезавершённый вопрос не учитывается."; font.family: "DejaVu Sans"; font.pixelSize: 17 }
        onAccepted: { selectionTimer.stop(); game.finish() }
    }
}
