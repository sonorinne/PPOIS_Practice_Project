#pragma once
#include <QObject>
#include <QVariantMap>
#include <QVariantList>
#include <QSettings>
#include "BoardSession.h"
#include "Database.h"
#include "Game.h"
#include "TrainingSession.h"

class GameController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantMap state READ state NOTIFY changed)
    Q_PROPERTY(QVariantList categories READ categories CONSTANT)
public:
    explicit GameController(QObject* parent = nullptr);
    QVariantMap state() const;
    QVariantList categories() const;
    Q_INVOKABLE void startClassic(const QString& name);
    Q_INVOKABLE void startTraining(const QString& name, const QVariantList& ids, int minCost, int maxCost, int count);
    Q_INVOKABLE void select(int cell);
    Q_INVOKABLE void catStake(int stake);
    Q_INVOKABLE void submit(const QString& answer);
    Q_INVOKABLE void advance();
    Q_INVOKABLE void nextBoard();
    Q_INVOKABLE void finish();
    Q_INVOKABLE void menu();
    Q_INVOKABLE void showHistory();
    Q_INVOKABLE void reviewMistakes();
    Q_INVOKABLE void setEffects(bool on);
signals:
    void changed();
private:
    void reset(const QString& name, bool training);
    void present();
    void saveResult();
    QString categoryName(int id) const;
    DataManager<Category> categoryData;
    DataManager<IQuestion> questionData;
    BoardSession board;
    Game game;
    TrainingSession trainingSession;
    std::shared_ptr<IQuestion> current;
    QSettings settings;
    QString screen = "menu", player = "Игрок", error, answerText;
    QVariantList history, mistakes;
    bool training = false, answered = false, correct = false, saved = false, effects = true;
    int pendingCell = -1, trainingScore = 0;
};
