#include <gtest/gtest.h>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickStyle>
#include <QTest>
#include <QTemporaryDir>
#include <QFontDatabase>
#include <QDir>
#include "GameController.h"

TEST(GuiController, ClassicAnswerCannotBeCountedTwiceAndHistoryPersists) {
    QSettings().clear(); GameController c;
    c.startClassic("  Тест  "); EXPECT_EQ(c.state()["player"].toString(), "Тест");
    c.select(0); if(c.state()["screen"]=="cat") c.catStake(500);
    ASSERT_EQ(c.state()["screen"].toString(),"question");
    c.submit(c.state()["primaryAnswer"].toString());
    EXPECT_TRUE(c.state()["correct"].toBool()); EXPECT_EQ(c.state()["answeredCount"].toInt(),1);
    c.submit("again"); EXPECT_EQ(c.state()["answeredCount"].toInt(),1);
    c.advance(); EXPECT_EQ(c.state()["screen"].toString(),"board");
    c.finish(); EXPECT_EQ(c.state()["history"].toList().size(),1);
    c.menu(); GameController reloaded; EXPECT_EQ(reloaded.state()["history"].toList().size(),1);
}
TEST(GuiController, TrainingFiltersScoresAndReviewsMistakes) {
    QSettings().clear(); GameController c;
    c.startTraining("Тест", {1}, 100, 100, 5);
    ASSERT_EQ(c.state()["total"].toInt(),1);
    c.submit(""); EXPECT_EQ(c.state()["score"].toInt(),0); c.advance();
    EXPECT_EQ(c.state()["screen"].toString(),"results"); EXPECT_EQ(c.state()["mistakes"].toList().size(),1);
    c.reviewMistakes(); EXPECT_EQ(c.state()["screen"].toString(),"question");
    c.submit(c.state()["primaryAnswer"].toString()); c.advance();
    EXPECT_EQ(c.state()["score"].toInt(),100); EXPECT_EQ(c.state()["correctCount"].toInt(),1);
    EXPECT_TRUE(c.state()["mistakes"].toList().isEmpty());
    c.startTraining("", {}, 500, 100, 5); EXPECT_FALSE(c.state()["error"].toString().isEmpty());
}
TEST(GuiController, RevealDoesNotDeductAndTrainingCountsError) {
    QSettings().clear(); GameController c;
    c.startClassic("Тест"); c.select(0); if(c.state()["screen"]=="cat") c.catStake(500);
    c.revealAnswer(); EXPECT_EQ(c.state()["score"].toInt(),0); EXPECT_EQ(c.state()["wrongCount"].toInt(),0);
    c.revealAnswer(); EXPECT_EQ(c.state()["score"].toInt(),0);
    c.startTraining("Тест",{1},100,200,2); auto serial=c.state()["serial"].toInt();
    c.revealAnswer(); EXPECT_EQ(c.state()["score"].toInt(),0); EXPECT_EQ(c.state()["wrongCount"].toInt(),1);
    c.advance(); EXPECT_GT(c.state()["serial"].toInt(),serial); EXPECT_FALSE(c.state()["answered"].toBool());
    c.setAudio(false,false,0.3,0.4); GameController restored;
    EXPECT_FALSE(restored.state()["music"].toBool()); EXPECT_FALSE(restored.state()["sounds"].toBool());
}
TEST(GuiView, LoadsAndSupportsMouseSelectionAndAllMediaLayouts) {
    QSettings().clear(); GameController c; c.setAudio(false,false,0.2,0.3);
    QQmlApplicationEngine engine;
    QStringList warnings;
    QObject::connect(&engine,&QQmlEngine::warnings,[&](const QList<QQmlError>& errors){ for(const auto& e:errors) warnings << e.toString(); });
    engine.rootContext()->setContextProperty("game",&c);
    engine.load(QUrl("qrc:/gui/qml/Main.qml"));
    ASSERT_EQ(engine.rootObjects().size(),1);
    auto window=qobject_cast<QQuickWindow*>(engine.rootObjects().front()); ASSERT_NE(window,nullptr);
    auto capture=[&](const QString& name){
        QTest::qWait(500);
        auto directory=qEnvironmentVariable("OWNGAME_SCREENSHOT_DIR");
        if(!directory.isEmpty()) { QDir().mkpath(directory); EXPECT_TRUE(window->grabWindow().save(directory+"/"+name+".png")); }
    };
    capture("01-menu");
    QTest::mouseClick(window,Qt::LeftButton,Qt::NoModifier,QPoint(1150,40)); capture("11-settings"); QTest::keyClick(window,Qt::Key_Escape);
    window->setProperty("page","setup"); capture("10-training-setup");
    QTest::mouseClick(window,Qt::LeftButton,Qt::NoModifier,QPoint(75,238));
    QTest::mouseClick(window,Qt::LeftButton,Qt::NoModifier,QPoint(200,618)); QTest::qWait(150);
    ASSERT_EQ(c.state()["screen"].toString(),"question");
    EXPECT_EQ(c.state()["theme"].toString(),QString::fromUtf8("Эдем")); c.menu();
    QTest::mouseClick(window,Qt::LeftButton,Qt::NoModifier,QPoint(180,544));
    ASSERT_EQ(c.state()["screen"].toString(),"board"); capture("02-board");
    QTest::mouseClick(window,Qt::LeftButton,Qt::NoModifier,QPoint(486,194)); QTest::qWait(650);
    if(c.state()["screen"]=="cat") { capture("03-cat"); c.catStake(100); }
    EXPECT_EQ(c.state()["screen"].toString(),"question"); capture("04-question");
    c.submit(""); capture("05-answer"); c.advance(); c.finish(); capture("06-results"); c.menu();
    c.startTraining("Тест",{1},100,200,2);
    auto field=window->findChild<QObject*>("answerInput"); ASSERT_NE(field,nullptr);
    field->setProperty("text","previous answer"); c.submit(""); c.advance();
    EXPECT_TRUE(field->property("text").toString().isEmpty()); c.menu();
    for (int id : {16,17}) {
        c.startTraining("Тест",{id},100,100,1);
        ASSERT_EQ(c.state()["screen"].toString(),"question");
        EXPECT_EQ(c.state()["kind"].toString(),id==17?"audio":"image");
        capture(id==17?"07-audio":"08-image");
        c.submit(c.state()["primaryAnswer"].toString()); c.advance(); c.menu();
    }
    c.startClassic("Тест");
    for(int cell=0; cell<30; ++cell) {
        c.select(cell);
        if(c.state()["screen"]=="cat") { capture("03-cat"); c.catStake(500); break; }
        c.submit(""); c.advance();
    }
    c.finish(); c.showHistory(); capture("09-history");
    EXPECT_TRUE(warnings.isEmpty()) << warnings.join("\n").toStdString();
}
int main(int argc,char** argv) {
    QGuiApplication app(argc,argv);
    QCoreApplication::setOrganizationName("OwnGameGuiTests");
    QCoreApplication::setApplicationName("Tests");
    QTemporaryDir config;
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,config.path());
    QQuickStyle::setStyle("Basic");
    QFontDatabase::addApplicationFont(":/gui/assets/Oswald.ttf");
    QGuiApplication::setFont(QFont("Oswald"));
    testing::InitGoogleTest(&argc,argv); return RUN_ALL_TESTS();
}
