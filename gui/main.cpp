#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFontDatabase>
#include <QFont>
#include <QIcon>
#include <QQuickStyle>
#include <QTimer>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QImage>
#include "GameController.h"
#include "Interaction.h"
int main(int argc, char** argv) {
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("OwnGameStudentProject");
    QCoreApplication::setApplicationName("OwnGameGui");
    QQuickStyle::setStyle("Basic");
    app.setWindowIcon(QIcon(":/gui/assets/cat.png"));
    QFontDatabase::addApplicationFont(":/gui/assets/Oswald.ttf");
    QGuiApplication::setFont(QFont("Oswald"));
    try {
        GameController controller;
        QQmlApplicationEngine engine;
        engine.rootContext()->setContextProperty("game", &controller);
        engine.load(QUrl("qrc:/gui/qml/Main.qml"));
        if (engine.rootObjects().isEmpty()) return 1;
        new Interaction(qobject_cast<QQuickWindow*>(engine.rootObjects().front()));
        int iconArg = app.arguments().indexOf("--export-icon");
        if (iconArg >= 0 && iconArg + 1 < app.arguments().size()) {
            const auto path = app.arguments().at(iconArg + 1);
            engine.rootObjects().front()->setProperty("exportingIcon", true);
            QTimer::singleShot(350, &app, [&engine, &app, path] {
                auto cat = engine.rootObjects().front()->findChild<QQuickItem*>("exportIcon");
                if (!cat) { app.exit(2); return; }
                auto image = cat->grabToImage(QSize(512,512));
                if (!image) { app.exit(3); return; }
                QObject::connect(image.data(), &QQuickItemGrabResult::ready, &app, [image, path, &app] {
                    app.exit(image->image().save(path) ? 0 : 4);
                });
            });
        }
        if (app.arguments().contains("--smoke-test")) QTimer::singleShot(1500, &app, &QCoreApplication::quit);
        return app.exec();
    } catch (const std::exception& e) { qCritical("%s", e.what()); return 1; }
}
