#include <gtest/gtest.h>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickItem>
#include <QQuickStyle>
#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFontDatabase>
#include <QDir>
#include <QColor>
#include <QCollator>
#include <QLocale>
#include "GameController.h"
#include "Interaction.h"

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
    c.startTraining("Тест", {1}, 100, 100, 1);
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
TEST(GuiController, CompletedRoundAdvancesAutomaticallyAndRevealHasOwnCue) {
    QSettings().clear(); GameController c;
    QSignalSpy started(&c,&GameController::roundStarted), ended(&c,&GameController::roundFinished);
    QSignalSpy revealed(&c,&GameController::answerRevealed), wrong(&c,&GameController::responseSubmitted);
    c.startClassic("Тест"); ASSERT_EQ(started.count(),1);
    for(int cell=0;cell<30;++cell) {
        c.select(cell); if(c.state()["screen"]=="cat") c.catStake(100);
        c.revealAnswer(); c.advance();
    }
    EXPECT_EQ(revealed.count(),30); EXPECT_EQ(wrong.count(),0); EXPECT_EQ(ended.count(),1);
    EXPECT_EQ(c.state()["screen"].toString(),"roundEnd");
    QTest::qWait(3000);
    EXPECT_EQ(c.state()["round"].toInt(),2); EXPECT_EQ(c.state()["screen"].toString(),"board");
    EXPECT_EQ(started.count(),2);
    c.startTraining("Тест",{1},100,100,1); c.revealAnswer();
    EXPECT_EQ(wrong.count(),0); EXPECT_EQ(c.state()["wrongCount"].toInt(),1);
}
TEST(GuiController, SortedThemesAndIndependentDisplaySettingsAndModeCues) {
    QSettings().clear(); GameController c;
    auto themes=c.categories(); QCollator collator(QLocale("ru_RU")); collator.setCaseSensitivity(Qt::CaseInsensitive); collator.setIgnorePunctuation(true);
    for(int i=1;i<themes.size();++i) EXPECT_LE(collator.compare(themes[i-1].toMap()["name"].toString(), themes[i].toMap()["name"].toString()),0);
    c.setEffects(false); c.setCurvature(true);
    GameController restored; EXPECT_FALSE(restored.state()["effects"].toBool()); EXPECT_TRUE(restored.state()["curvature"].toBool());
    QSignalSpy start(&c,&GameController::trainingStarted), end(&c,&GameController::modeFinished);
    c.startTraining("Тест",{1},100,100,1); EXPECT_EQ(start.count(),1); c.menu(); EXPECT_EQ(end.count(),1);
    c.startClassic("Тест"); c.finish(); EXPECT_EQ(end.count(),2); c.menu(); EXPECT_EQ(end.count(),2);
}
TEST(GuiController, TrainingCapacityAndFarewellPreserveAnswers) {
    QSettings().clear(); GameController c;
    EXPECT_EQ(c.availableQuestions({1},100,100),1);
    c.startTraining("Тест",{1},100,100,2); EXPECT_EQ(c.state()["screen"].toString(),"menu"); EXPECT_FALSE(c.state()["error"].toString().isEmpty());
    c.startTraining("Тест",{1},100,100,1); c.submit(""); c.shutdown();
    EXPECT_EQ(c.state()["screen"].toString(),"farewell"); EXPECT_EQ(c.state()["history"].toList().size(),1);
    for(auto pos:{QPointF(80,50),QPointF(640,400),QPointF(1100,700)}) {
        auto roundtrip=Interaction::sourcePoint(Interaction::displayPoint(pos,QSizeF(1280,800)),QSizeF(1280,800));
        EXPECT_NEAR(roundtrip.x(),pos.x(),0.001); EXPECT_NEAR(roundtrip.y(),pos.y(),0.001);
    }
}
TEST(GuiController, NormalTrainingAndClassicFinishShowResultsButShutdownShowsFarewell) {
    QSettings().clear(); GameController c;
    c.startTraining("Тест",{1},100,100,1); c.submit(""); c.advance();
    EXPECT_EQ(c.state()["screen"].toString(),"results");
    c.startClassic("Тест"); c.finish(); EXPECT_EQ(c.state()["screen"].toString(),"results");
    c.startTraining("Тест",{1},100,100,1); c.shutdown(); EXPECT_EQ(c.state()["screen"].toString(),"farewell");
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
    new Interaction(window);
    auto capture=[&](const QString& name){
        QTest::qWait(500);
        auto directory=qEnvironmentVariable("OWNGAME_SCREENSHOT_DIR");
        if(!directory.isEmpty()) { QDir().mkpath(directory); EXPECT_TRUE(window->grabWindow().save(directory+"/"+name+".png")); }
    };
    auto click=[&](const char* name) {
        auto item=window->findChild<QQuickItem*>(name); EXPECT_NE(item,nullptr);
        if(item) { auto point=item->mapToScene(QPointF(item->width()/2,item->height()/2)); if(window->property("curvatureInput").toBool() && !window->property("modalOpen").toBool()) point=Interaction::displayPoint(point,window->size()); QTest::mouseClick(window,Qt::LeftButton,Qt::NoModifier,point.toPoint()); }
    };
    if(qEnvironmentVariableIsSet("OWNGAME_TEST_CURVATURE")) c.setCurvature(true);
    capture("01-menu");
    QTest::qWait(900); capture("01-menu-warm");
    auto cat=window->findChild<QObject*>("menuCat"); ASSERT_NE(cat,nullptr);
    c.setEffects(false); QSignalSpy tailMotion(cat,SIGNAL(tailChanged())); QTest::qWait(180);
    EXPECT_TRUE(cat->property("animated").toBool()); EXPECT_GT(tailMotion.count(),0); c.setEffects(true);
    auto nameField=window->findChild<QQuickItem*>("playerNameInput"); ASSERT_NE(nameField,nullptr);
    nameField->forceActiveFocus(); nameField->setProperty("text",""); QCoreApplication::processEvents();
    auto hint=window->findChild<QQuickItem*>("nameHint"); ASSERT_NE(hint,nullptr); EXPECT_TRUE(hint->isVisible());
    auto title=window->findChild<QQuickItem*>("menuTitle"); auto play=window->findChild<QQuickItem*>("playButton");
    ASSERT_NE(title,nullptr); ASSERT_NE(play,nullptr);
    EXPECT_NEAR(title->mapToScene(QPointF(title->width()/2,0)).x(),play->mapToScene(QPointF(play->width()/2,0)).x(),1);
    click("settingsButton"); capture("11-settings");
    for(const char* sliderName : {"musicSlider","soundSlider"}) {
        auto slider=window->findChild<QObject*>(sliderName); ASSERT_NE(slider,nullptr);
        auto handle=slider->property("handle").value<QQuickItem*>(); ASSERT_NE(handle,nullptr);
        QTest::mouseMove(window,handle->mapToScene(QPointF(handle->width()/2,handle->height()/2)).toPoint()); QTest::qWait(160);
        EXPECT_EQ(handle->property("color").value<QColor>(),QColor("#ffffff"));
    }
    QTest::keyClick(window,Qt::Key_Escape);
    window->setProperty("page","setup"); capture("10-training-setup");
    QPointF themePoint(75,282); if(window->property("curvatureInput").toBool()) themePoint=Interaction::displayPoint(themePoint,window->size()); QTest::mouseClick(window,Qt::LeftButton,Qt::NoModifier,themePoint.toPoint());
    window->findChild<QObject*>("questionCount")->setProperty("currentIndex",0);
    click("startTrainingButton"); QTest::qWait(150);
    ASSERT_EQ(c.state()["screen"].toString(),"question");
    EXPECT_EQ(c.state()["theme"].toString(),c.categories().first().toMap()["name"].toString()); c.menu();
    play->forceActiveFocus(); QTest::keyClick(window,Qt::Key_Return);
    ASSERT_EQ(c.state()["screen"].toString(),"board"); QTest::qWait(2300); capture("02-board");
    QPointF cellPoint(486,232); if(window->property("curvatureInput").toBool()) cellPoint=Interaction::displayPoint(cellPoint,window->size()); QTest::mouseClick(window,Qt::LeftButton,Qt::NoModifier,cellPoint.toPoint()); QTest::qWait(650);
    if(c.state()["screen"]=="cat") { capture("03-cat"); c.catStake(100); }
    EXPECT_EQ(c.state()["screen"].toString(),"question"); capture("04-question");
    c.submit(""); capture("05-answer"); c.advance(); c.finish(); capture("06-results"); c.menu();
    c.startTraining("Тест",{1},100,200,2);
    auto row=window->findChild<QQuickItem*>("answerRow"); ASSERT_NE(row,nullptr);
    EXPECT_NEAR(row->x()+row->width()/2,584.0,1.0);
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
    click("exitButton"); capture("12-exit-confirmation"); QTest::keyClick(window,Qt::Key_Escape);
    c.finish(); c.showHistory(); capture("09-history");
    c.startClassic("Тест"); QTest::keyClick(window,Qt::Key_Escape); QTest::qWait(250);
    EXPECT_TRUE(window->property("modalOpen").toBool()); QTest::keyClick(window,Qt::Key_Escape);
    c.menu(); click("exitButton"); QTest::qWait(250);
    auto exitDialog=window->findChild<QObject*>("exitDialog"); ASSERT_NE(exitDialog,nullptr);
    QMetaObject::invokeMethod(exitDialog,"accept");
    EXPECT_EQ(c.state()["screen"].toString(),"farewell"); capture("14-farewell");
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
