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
    property int hoveredRow: -1
    property var chosenCategories: []
    property string playerName: s.player
    property string backendScreen: "menu"
    Shortcut { sequence: "Return"; enabled: page === "question" && window.s.answered; onActivated: game.advance() }
    font.family: "Tiny5"
    font.pixelSize: 24
    Connections {
        target: game
        function onChanged() { if (game.state.screen !== window.backendScreen) { window.backendScreen = game.state.screen; window.page = game.state.screen } }
    }
    onPageChanged: { reveal.restart(); answerInput.text = ""; selectedCell = -1; hoveredRow = -1; audio.stop(); if(page === "question") Qt.callLater(function() { answerInput.forceActiveFocus() }) }
    Item {
        id: stage
        width: 1280; height: 800
        anchors.centerIn: parent
        scale: Math.min(window.width / width, window.height / height)
        clip: true
        Television { anchors.fill: parent; effects: window.s.effects }
        Rectangle { x: 38; y: 38; width: 38; height: 5; color: "#e3d28a" }
        Text { x: 90; y: 27; text: "СВОЯ ИГРА / ИНТЕЛЛЕКТУАЛЬНЫЙ ТРЕНАЖЁР"; color: "#b2c6df"; font.pixelSize: 21; font.letterSpacing: 1.1 }
        Row {
            anchors.right: parent.right; anchors.rightMargin: 44; y: 22; spacing: 16
            CheckBox { id: effectToggle; text: "ЭФИР / CRT"; checked: window.s.effects; font.pixelSize: 19; palette.windowText: "#adbbd6"; onClicked: game.setEffects(checked) }
            RetroButton { text: "МЕНЮ"; implicitHeight: 38; implicitWidth: 100; font.pixelSize: 20; visible: page !== "menu"; onClicked: { if (["board", "question", "cat"].indexOf(page) >= 0) leaveDialog.open(); else { game.menu(); page = "menu" } } }
        }
        Rectangle { x: 44; y: 72; width: 1192; height: 1; color: "#4a5b85" }

        Item {
            id: content
            x: 56; y: 100; width: 1168; height: 590
            ParallelAnimation { id: reveal; NumberAnimation { target: content; property: "opacity"; from: 0; to: 1; duration: window.s.effects ? 240 : 0 } NumberAnimation { target: content; property: "scale"; from: 0.975; to: 1; duration: window.s.effects ? 300 : 0; easing.type: Easing.OutCubic } }
            Item {
                anchors.fill: parent; visible: page === "menu"
                Text { y: 12; text: "ДОБРО ПОЖАЛОВАТЬ В ЭФИР"; color: "#e1ce85"; font.pixelSize: 22; font.letterSpacing: 2 }
                Text { y: 62; text: "СВОЯ\nИГРА"; lineHeight: 0.82; font.pixelSize: 132; font.letterSpacing: 6; color: "#edf4df"; style: Text.Raised; styleColor: "#446ca1" }
                Text { y: 310; text: "Один игрок. Шесть тем. Твой ход."; color: "#9cb6d9"; font.pixelSize: 26 }
                TextField { id: nameInput; x: 0; y: 348; width: 390; height: 49; text: window.playerName; placeholderText: "Как тебя представить?"; maximumLength: 24; color: "#f2edcf"; font.pixelSize: 25; onTextEdited: window.playerName = text; background: Rectangle { color: "#081635"; border.color: "#566c99" } }
                Column { y: 418; spacing: 12
                    Row { spacing: 12
                        RetroButton { text: "К ТАБЛО  →"; accent: true; implicitWidth: 250; onClicked: game.startClassic(nameInput.text) }
                        RetroButton { text: "ТРЕНИРОВКА"; implicitWidth: 250; onClicked: page = "setup" }
                    }
                    RetroButton { text: "АРХИВ РЕЗУЛЬТАТОВ"; implicitWidth: 512; onClicked: game.showHistory() }
                }
                Rectangle { x: 681; y: 40; width: 445; height: 478; color: "#07152c"; opacity: 0.4; border.color: "#6f819c" }
                Text { x: 710; y: 61; text: "КАФЕДРА СЛУЧАЙНЫХ ЗНАНИЙ"; color: "#8296b2"; font.pixelSize: 19 }
                Cat { x: 717; y: 126; width: 390; height: 410; animated: window.s.effects }
                Text { x: 716; y: 539; text: "Не знаешь? Это повод узнать."; color: "#d3c886"; font.pixelSize: 23 }
            }
            Item {
                anchors.fill: parent; visible: page === "board"
                Text { text: "ТАБЛО / РАУНД " + window.s.round; color: "#e4d590"; font.pixelSize: 29 }
                Text { anchors.right: parent.right; y: 5; text: "ВЫБЕРИ ТЕМУ И СТОИМОСТЬ"; color: "#91a8cc"; font.pixelSize: 21 }
                Column {
                    y: 58; width: parent.width; spacing: 0
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
                                color: window.hoveredRow === boardRow.rowNumber || (window.selectedCell >= 0 && Math.floor(window.selectedCell / 5) === boardRow.rowNumber) ? "#646039" : "#142b71"
                                border.color: "#566caa"
                                Text { anchors.fill: parent; anchors.margins: 14; text: boardRow.modelData.name.toUpperCase(); color: "#e1d597"; font.pixelSize: 25; wrapMode: Text.WordWrap; verticalAlignment: Text.AlignVCenter; lineHeight: 0.9 }
                            }
                            Repeater {
                                model: boardRow.modelData.cells
                                delegate: Rectangle {
                                    required property var modelData
                                    width: 164; height: 73
                                    color: window.selectedCell === modelData.index ? "#e5d99c" : cellMouse.containsMouse && !modelData.used ? "#244ba1" : "#132c78"
                                    border.color: activeFocus ? "#fff6d0" : "#566caa"
                                    border.width: activeFocus ? 3 : 1
                                    activeFocusOnTab: !modelData.used
                                    Accessible.role: Accessible.Button
                                    Accessible.name: boardRow.modelData.name + " " + modelData.cost
                                    Keys.onReturnPressed: if (!modelData.used && window.selectedCell < 0) { window.selectedCell = modelData.index; selectionTimer.restart() }
                                    Keys.onSpacePressed: if (!modelData.used && window.selectedCell < 0) { window.selectedCell = modelData.index; selectionTimer.restart() }
                                    Text { anchors.centerIn: parent; text: parent.modelData.used ? "" : parent.modelData.cost; font.pixelSize: 42; color: window.selectedCell === parent.modelData.index ? "#112455" : "#e8dea8" }
                                    MouseArea {
                                        id: cellMouse; anchors.fill: parent; hoverEnabled: true
                                        enabled: !parent.modelData.used && window.selectedCell < 0
                                        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                        onEntered: window.hoveredRow = boardRow.rowNumber
                                        onExited: window.hoveredRow = -1
                                        onClicked: { window.selectedCell = parent.modelData.index; selectionTimer.restart() }
                                    }
                                }
                            }
                        }
                    }
                }
                Row { y: 522; spacing: 16
                    RetroButton { text: window.s.canNextBoard ? "СЛЕДУЮЩИЕ ТЕМЫ →" : "К РЕЗУЛЬТАТАМ →"; visible: window.s.boardComplete; accent: true; onClicked: window.s.canNextBoard ? game.nextBoard() : game.finish() }
                    RetroButton { text: "ЗАВЕРШИТЬ СЕССИЮ"; onClicked: leaveDialog.open() }
                    Text { y: 13; text: "Темы в этой сессии не повторяются"; color: "#8fa6c6"; font.pixelSize: 22 }
                }
            }
            Item {
                anchors.fill: parent; visible: page === "cat"
                Cat { x: 82; y: 80; width: 350; height: 390; animated: window.s.effects }
                Column { x: 510; y: 110; spacing: 26; width: 640
                    Text { text: "КОТ В МЕШКЕ"; color: "#f0df99"; font.pixelSize: 64 }
                    Text { width: 630; wrapMode: Text.WordWrap; text: "Тема: " + window.s.catTheme + "\nСтоимость выбираешь ты."; color: "#c2cee2"; font.pixelSize: 30; lineHeight: 1.1 }
                    Row { spacing: 20; RetroButton { text: "100"; implicitWidth: 200; accent: true; onClicked: game.catStake(100) } RetroButton { text: "500"; implicitWidth: 200; onClicked: game.catStake(500) } }
                    Text { text: "Верный ответ: +стоимость. Ошибка: −стоимость."; color: "#8ca4c8"; font.pixelSize: 21 }
                }
            }
            Item {
                anchors.fill: parent; visible: page === "question"
                Text { text: (window.s.theme || "").toUpperCase() + " / " + (window.s.cost || "") + (window.s.training ? " / ТРЕНИРОВКА" : ""); color: "#dece85"; font.pixelSize: 25 }
                Item {
                    id: mediaArea; x: 300; y: 42; width: 568; height: 218
                    visible: window.s.kind !== "text"
                    Image { anchors.fill: parent; visible: window.s.kind === "image"; source: window.s.kind === "image" ? window.s.media : ""; fillMode: Image.PreserveAspectFit; smooth: false }
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
                    x: 115; y: window.s.kind === "text" ? 88 : 272; width: 938; height: window.s.kind === "text" ? 256 : 106
                    text: window.s.text || ""; color: "#f0e6b8"; font.family: "Tiny5"; font.pixelSize: window.s.kind === "text" ? 36 : 28
                    horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; wrapMode: Text.WordWrap; lineHeight: 0.96
                    fontSizeMode: Text.Fit; minimumPixelSize: 22
                }
                Row { x: 425; y: 236; spacing: 10; visible: window.s.kind === "audio" && !window.s.answered
                    RetroButton { text: audio.playbackState === MediaPlayer.PlayingState ? "ПАУЗА" : "СЛУШАТЬ"; implicitHeight: 32; font.pixelSize: 18; onClicked: audio.playbackState === MediaPlayer.PlayingState ? audio.pause() : audio.play() }
                    RetroButton { text: "СНАЧАЛА"; implicitHeight: 32; font.pixelSize: 18; onClicked: { audio.position = 0; audio.play() } }
                }
                Text { x: 115; y: 371; visible: window.s.kind === "audio" && audio.error !== MediaPlayer.NoError; text: "Не удалось воспроизвести аудио. Проверьте файл и устройство звука."; color: "#e9b398"; font.pixelSize: 20 }
                Row { x: 150; y: 407; spacing: 12; visible: !window.s.answered
                    TextField { id: answerInput; width: 635; height: 53; placeholderText: "Твой ответ…"; placeholderTextColor: "#a2b5cf"; color: "#eff0db"; font.family: "DejaVu Sans"; font.pixelSize: 22; selectByMouse: true; maximumLength: 250
                        background: Rectangle { color: "#081737"; border.color: answerInput.activeFocus ? "#e5d58b" : "#6c84b2" }
                        onAccepted: if (page === "question") { game.submit(text); audio.stop() }
                    }
                    RetroButton { text: "ОТВЕТИТЬ"; accent: true; onClicked: { game.submit(answerInput.text); audio.stop() } }
                }
                RetroButton { x: 470; y: 482; visible: !window.s.answered; text: "НЕ ЗНАЮ / ПОКАЗАТЬ ОТВЕТ"; font.pixelSize: 21; onClicked: { game.submit(""); audio.stop() } }
                Column { x: 100; y: 401; width: 968; spacing: 12; visible: window.s.answered
                    Text { width: parent.width; horizontalAlignment: Text.AlignHCenter; text: window.s.correct ? "ВЕРНО!" : "ОТВЕТ: " + (window.s.primaryAnswer || ""); color: window.s.correct ? "#b7dca0" : "#f0d394"; font.pixelSize: 31; wrapMode: Text.WordWrap }
                    Text { width: parent.width; horizontalAlignment: Text.AlignHCenter; text: "Твой ответ: " + (window.s.answer || "пропуск"); color: "#a5b8d5"; font.family: "DejaVu Sans"; font.pixelSize: 17; elide: Text.ElideRight }
                    RetroButton { anchors.horizontalCenter: parent.horizontalCenter; text: "ПРОДОЛЖИТЬ →"; accent: true; onClicked: game.advance() }
                }
            }
            Item {
                anchors.fill: parent; visible: page === "setup"
                Text { text: "ТРЕНИРОВКА"; color: "#eee2a6"; font.pixelSize: 56 }
                Text { y: 65; text: "Выбери темы или оставь всё пустым для смешанного набора."; color: "#9fb6d5"; font.pixelSize: 25 }
                ScrollView { y: 118; width: parent.width; height: 238; clip: true; contentWidth: availableWidth
                Flow { width: 1135; spacing: 8
                    Repeater { model: game.categories
                        CheckBox { required property var modelData; text: modelData.name; font.pixelSize: 23; palette.windowText: "#e2dcae"; checked: false
                            onClicked: { let ids = window.chosenCategories.slice(); let i = ids.indexOf(modelData.id); if(checked && i < 0) ids.push(modelData.id); else if(!checked && i >= 0) ids.splice(i,1); window.chosenCategories = ids }
                        }
                    }
                }
                }
                Row { y: 378; spacing: 26
                    Column { spacing: 8; Text { text: "ОТ СТОИМОСТИ"; color: "#a4b9d4"; font.pixelSize: 20 } ComboBox { id: minCost; model: [100,200,300,400,500]; width: 190; font.pixelSize: 24 } }
                    Column { spacing: 8; Text { text: "ДО СТОИМОСТИ"; color: "#a4b9d4"; font.pixelSize: 20 } ComboBox { id: maxCost; model: [100,200,300,400,500]; currentIndex: 4; width: 190; font.pixelSize: 24 } }
                    Column { spacing: 8; Text { text: "ВОПРОСОВ"; color: "#a4b9d4"; font.pixelSize: 20 } ComboBox { id: questionCount; model: [5,10,15,20,30]; currentIndex: 1; width: 190; font.pixelSize: 24 } }
                }
                RetroButton { y: 492; text: "НАЧАТЬ ТРЕНИРОВКУ →"; accent: true; enabled: minCost.currentIndex <= maxCost.currentIndex; onClicked: game.startTraining(window.playerName, window.chosenCategories, Number(minCost.currentText), Number(maxCost.currentText), Number(questionCount.currentText)) }
                Text { x: 470; y: 502; text: "Без штрафа за ошибки. С повторением трудных вопросов."; color: "#97aece"; font.pixelSize: 23 }
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
        Text { x: 56; y: 698; width: 1168; text: window.s.error; color: "#f3b2a1"; font.pixelSize: 20; elide: Text.ElideRight }
        Rectangle { x: 44; y: 730; width: 1192; height: 1; color: "#4a5b85" }
        Row { x: 56; y: 751; spacing: 40
            Text { text: window.s.player.toUpperCase(); color: "#e7d791"; font.pixelSize: 25 }
            Text { text: "СЧЁТ  " + window.s.score; color: "#edf1de"; font.pixelSize: 25 }
            Text { text: "ВЕРНО  " + window.s.correctCount + "   /   ОШИБКИ  " + window.s.wrongCount; color: "#9cb5d5"; font.pixelSize: 23 }
        }
        Text { anchors.right: parent.right; anchors.rightMargin: 56; y: 754; text: page === "question" && window.s.training ? "ВОПРОС " + window.s.number + " / " + window.s.total : page === "board" ? "ОТВЕЧЕНО  " + window.s.answeredCount : "ЛИЧНЫЙ ЭФИР · 01"; color: "#839dc3"; font.pixelSize: 21 }
    }
    Timer { id: selectionTimer; interval: window.s.effects ? 420 : 1; onTriggered: { if(page === "board") game.select(window.selectedCell) } }
    AudioOutput { id: audioOutput; volume: 0.7 }
    MediaPlayer { id: audio; audioOutput: audioOutput; source: page === "question" && window.s.kind === "audio" ? window.s.media : ""; onSourceChanged: { if(source.toString().length > 0) play() } }
    Dialog {
        id: leaveDialog; anchors.centerIn: parent; width: 520; modal: true; title: "Завершить сессию?"; standardButtons: Dialog.Ok | Dialog.Cancel
        Label { text: "Результаты ответов сохранятся в архиве.\nНезавершённый вопрос не учитывается."; font.family: "DejaVu Sans"; font.pixelSize: 17 }
        onAccepted: { selectionTimer.stop(); game.finish() }
    }
}
