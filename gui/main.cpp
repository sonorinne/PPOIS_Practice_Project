#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFontDatabase>
#include <QFont>
#include <QQuickStyle>
#include <QTimer>
#include "GameController.h"
int main(int argc, char** argv) {
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("OwnGameStudentProject");
    QCoreApplication::setApplicationName("OwnGameGui");
    QQuickStyle::setStyle("Basic");
    QFontDatabase::addApplicationFont(":/gui/assets/Oswald.ttf");
    QGuiApplication::setFont(QFont("Oswald"));
    try {
        GameController controller;
        QQmlApplicationEngine engine;
        engine.rootContext()->setContextProperty("game", &controller);
        engine.load(QUrl("qrc:/gui/qml/Main.qml"));
        if (engine.rootObjects().isEmpty()) return 1;
        if (app.arguments().contains("--smoke-test")) QTimer::singleShot(1500, &app, &QCoreApplication::quit);
        return app.exec();
    } catch (const std::exception& e) { qCritical("%s", e.what()); return 1; }
}
