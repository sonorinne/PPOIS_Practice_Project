#include "GameController.h"
#include "AudioQuestion.h"
#include "ImageQuestion.h"
#include <QCoreApplication>
#include <QDir>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUrl>
#include <QCollator>
#include <QLocale>
#include <algorithm>

GameController::GameController(QObject* parent) : QObject(parent) {
    roundTimer.setSingleShot(true);
    connect(&roundTimer, &QTimer::timeout, this, [this] {
        if(screen != "roundEnd") return;
        if(board.NextBoard()) { screen = "board"; emit changed(); emit roundStarted(board.GetRound()); }
        else finish();
    });
    Database db(":memory:"); // Seed content only; personal history is stored separately.
    db.Initialize(); db.LoadCategories(categoryData); db.LoadQuestions(questionData);
    music = settings.value("music", true).toBool(); sounds = settings.value("sounds", true).toBool();
    musicVolume = settings.value("musicVolume", 0.22).toDouble(); soundVolume = settings.value("soundVolume", 0.6).toDouble();
    curvature = settings.value("curvature", false).toBool();
    effects = settings.value("effects", true).toBool();
    player = settings.value("player", "Игрок").toString();
    history = QJsonDocument::fromJson(settings.value("history").toByteArray()).array().toVariantList();
}
QVariantList GameController::categories() const {
    QVariantList result;
    for (const auto& c : categoryData.GetAll()) result << QVariantMap{{"id", c->GetId()}, {"name", QString::fromStdString(c->GetName())}};
    QCollator collator(QLocale("ru_RU")); collator.setCaseSensitivity(Qt::CaseInsensitive); collator.setIgnorePunctuation(true);
    std::stable_sort(result.begin(),result.end(),[&](const QVariant& a,const QVariant& b) {
        return collator.compare(a.toMap()["name"].toString(),b.toMap()["name"].toString()) < 0;
    });
    return result;
}
QString GameController::categoryName(int id) const { return QString::fromStdString(categoryData.GetById(id)->GetName()); }
QVariantMap GameController::state() const {
    const int good = training ? trainingSession.GetStatistics().GetTotal().correct : game.GetSession().GetCorrectCount();
    const int bad = training ? trainingSession.GetStatistics().GetTotal().total - good : game.GetSession().GetIncorrectCount();
    QVariantMap s{{"curvature", curvature}, {"serial", questionSerial}, {"music", music}, {"sounds", sounds}, {"musicVolume", musicVolume}, {"soundVolume", soundVolume}, {"screen", screen}, {"player", player}, {"effects", effects}, {"training", training},
        {"score", training ? trainingScore : game.GetSession().GetScore()}, {"correctCount", good}, {"wrongCount", bad},
        {"answeredCount", good + bad}, {"round", board.GetRound()}, {"answered", answered}, {"correct", correct},
        {"catTheme", QString::fromStdString(board.GetCatTheme())}, {"error", error}, {"answer", answerText}, {"history", history}, {"mistakes", mistakes},
        {"canNextBoard", board.HasNextBoard()}, {"boardComplete", board.IsComplete()},
        {"total", training ? trainingSession.GetQuestionCount() : 30}, {"number", training ? qMin(trainingSession.GetCurrentNumber(), trainingSession.GetQuestionCount()) : good + bad + 1}};
    QVariantList rows;
    int rowIndex = 0;
    for (const auto& row : board.GetRows()) {
        QVariantList cells;
        for (int col = 0; col < 5; ++col) cells << QVariantMap{{"index", rowIndex * 5 + col}, {"cost", (col + 1) * 100}, {"used", board.IsUsed(rowIndex * 5 + col)}};
        rows << QVariantMap{{"name", QString::fromStdString(row.name)}, {"cells", cells}};
        ++rowIndex;
    }
    s["rows"] = rows;
    if (current) {
        auto mediaQuestion = current;
        if (auto cat = std::dynamic_pointer_cast<CatQuestion>(current)) mediaQuestion = cat->GetSource();
        QString kind = "text", path;
        if (auto audio = std::dynamic_pointer_cast<AudioQuestion>(mediaQuestion)) { kind = "audio"; path = QString::fromStdString(audio->GetAudioPath()); }
        if (auto image = std::dynamic_pointer_cast<ImageQuestion>(mediaQuestion)) { kind = "image"; path = QString::fromStdString(image->GetImagePath()); }
        s["kind"] = kind; s["text"] = QString::fromStdString(current->GetText());
        s["theme"] = categoryName(current->GetCategoryId()); s["cost"] = current->GetCost();
        s["primaryAnswer"] = QString::fromStdString(current->GetPrimaryAnswer());
        s["media"] = path.isEmpty() ? QString() : QUrl::fromLocalFile(QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(path)).toString();
    }
    return s;
}
void GameController::reset(const QString& name, bool isTraining) {
    roundTimer.stop();
    player = name.trimmed().isEmpty() ? "Игрок" : name.trimmed().left(24);
    settings.setValue("player", player);
    training = isTraining; game = Game(); trainingSession = TrainingSession(); current.reset();
    error.clear(); mistakes.clear(); answerText.clear(); trainingScore = 0; saved = false; answered = false;
}
void GameController::startClassic(const QString& name) {
    reset(name, false);
    board.Reset(categoryData.GetAll(), questionData.GetAll());
    if (board.NextBoard()) screen = "board";
    else { screen = "menu"; error = "Для раунда нужны шесть полных тем табло и три темы для котов."; }
    emit changed();
    if(screen == "board") emit roundStarted(board.GetRound());
}
int GameController::availableQuestions(const QVariantList& ids,int minCost,int maxCost) const {
    TrainingSettings config; config.min_cost=minCost; config.max_cost=maxCost;
    for(const auto& id:ids) config.category_ids.push_back(id.toInt());
    return static_cast<int>(FilterQuestions(questionData.GetAll(),config).size());
}
void GameController::startTraining(const QString& name, const QVariantList& ids, int minCost, int maxCost, int count) {
    int maximum=availableQuestions(ids,minCost,maxCost);
    if(count>maximum || maximum==0) { error=QString("Доступно вопросов: %1. Выберите не больше этого количества.").arg(maximum); emit changed(); return; }
    reset(name, true);
    TrainingSettings config; config.min_cost = minCost; config.max_cost = maxCost; config.question_count = count;
    for (const auto& id : ids) config.category_ids.push_back(id.toInt());
    try { trainingSession.Generate(questionData.GetAll(), config); current = trainingSession.GetCurrentQuestion(); present(); emit trainingStarted(); }
    catch (const std::exception& e) { error = QString::fromUtf8(e.what()); emit changed(); }
}
void GameController::select(int cell) {
    if (screen != "board") return;
    try {
        pendingCell = cell;
        if (board.IsCatSlot(cell) && !board.IsUsed(cell)) { screen = "cat"; emit changed(); emit catOpened(); return; }
        current = board.Select(cell); present();
    } catch (const std::exception& e) { error = QString::fromUtf8(e.what()); emit changed(); }
}
void GameController::catStake(int stake) {
    if (screen != "cat") return;
    try { current = board.Select(pendingCell, stake); present(); }
    catch (const std::exception& e) { error = QString::fromUtf8(e.what()); emit changed(); }
}
void GameController::present() { ++questionSerial; answered = false; error.clear(); answerText.clear(); screen = "question"; emit changed(); emit questionOpened(); }
void GameController::submit(const QString& input) {
    if (screen != "question" || answered || !current) return;
    auto text = input.trimmed().normalized(QString::NormalizationForm_C);
    // Existing questions own accepted aliases. Try case variants without fuzzy guessing.
    QStringList candidates{text, text.toLower(), text.toUpper()};
    if (!text.isEmpty()) { auto capital = text.toLower(); capital[0] = capital[0].toUpper(); candidates << capital; }
    QString accepted = text;
    for (const auto& candidate : candidates) if (!candidate.isEmpty() && current->CheckAnswer(candidate.toStdString())) { accepted = candidate; break; }
    if (training) {
        correct = trainingSession.SubmitAnswer(accepted.toStdString());
        if (correct) trainingScore += current->GetCost();
    } else correct = game.AnswerQuestion(current, accepted.toStdString());
    answerText = text; answered = true;
    if (!correct) mistakes << QVariantMap{{"theme", categoryName(current->GetCategoryId())}, {"text", QString::fromStdString(current->GetText())}, {"answer", QString::fromStdString(current->GetPrimaryAnswer())}};
    emit changed(); if(!revealing) emit responseSubmitted(correct);
}
void GameController::revealAnswer() {
    if (screen != "question" || answered || !current) return;
    if (training) { revealing = true; submit(""); revealing = false; emit answerRevealed(); return; }
    correct = false; answered = true; answerText.clear();
    emit changed(); emit answerRevealed();
}
void GameController::advance() {
    if (screen != "question" || !answered) return;
    current.reset();
    if (training) {
        if (trainingSession.IsFinished()) { finish(); return; }
        current = trainingSession.GetCurrentQuestion(); present();
    } else if(board.IsComplete()) { screen = "roundEnd"; emit changed(); emit roundFinished(); roundTimer.start(2800); }
    else { screen = "board"; emit changed(); }
}
void GameController::nextBoard() {
    if (screen != "board" || !board.IsComplete()) return;
    if (!board.NextBoard()) { finish(); return; }
    emit changed();
}
void GameController::saveResult() {
    if (saved) return;
    auto s = state();
    history.prepend(QVariantMap{{"date", QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm")}, {"player", player},
        {"mode", training ? "Тренировка" : "Игра"}, {"score", s["score"]}, {"correct", s["correctCount"]}, {"total", s["answeredCount"]}, {"mistakes", mistakes}});
    while (history.size() > 50) history.removeLast();
    settings.setValue("history", QJsonDocument(QJsonArray::fromVariantList(history)).toJson(QJsonDocument::Compact));
    saved = true;
}
void GameController::finish() { if(screen == "board" || screen == "question" || screen == "cat") emit modeFinished(); roundTimer.stop(); if (screen == "board" || screen == "question" || screen == "cat" || screen == "roundEnd") saveResult(); current.reset(); screen = "results"; emit changed(); }
void GameController::menu() { if(screen == "board" || screen == "question" || screen == "cat") emit modeFinished(); roundTimer.stop(); if (screen == "board" || screen == "question" || screen == "cat" || screen == "roundEnd") saveResult(); current.reset(); error.clear(); screen = "menu"; emit changed(); }
void GameController::showHistory() { screen = "history"; emit changed(); }
void GameController::reviewMistakes() {
    if (!training || screen != "results" || trainingSession.GetMistakes().empty()) return;
    trainingSession = trainingSession.CreateMistakesReview(); mistakes.clear(); trainingScore = 0; saved = false;
    current = trainingSession.GetCurrentQuestion(); present();
}
void GameController::setEffects(bool on) { effects = on; settings.setValue("effects", on); emit changed(); }

void GameController::setAudio(bool m, bool s, double mv, double sv) {
 music=m; sounds=s; musicVolume=qBound(0.0,mv,1.0); soundVolume=qBound(0.0,sv,1.0);
 settings.setValue("music",music); settings.setValue("sounds",sounds); settings.setValue("musicVolume",musicVolume); settings.setValue("soundVolume",soundVolume); emit changed();
}

void GameController::setCurvature(bool on) { curvature=on; settings.setValue("curvature",on); emit changed(); }

void GameController::shutdown() { roundTimer.stop(); if(screen=="board" || screen=="question" || screen=="cat" || screen=="roundEnd") saveResult(); current.reset(); screen="farewell"; emit changed(); emit modeFinished(); }
