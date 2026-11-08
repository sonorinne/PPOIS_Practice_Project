#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <map>
#include <algorithm>

#include "Game.h"
#include "Category.h"
#include "DataManager.h"
#include "GameSession.h"
#include "StandardQuestion.h"
#include "ImageQuestion.h"
#include "AudioQuestion.h"
#include "Database.h"
#include "AssetsWork.h"
#include "TrainingSession.h"
#include "Statistics.h"
#include "RecommendationSystem.h"

namespace fs = std::filesystem;

TEST(CategoryTest, BasicGetters) {
    std::string name = "История";
    Category cat(1, name);
    EXPECT_EQ(cat.GetId(), 1);
    EXPECT_EQ(cat.GetName(), "История");
}

TEST(CategoryTest, DifferentIdsAndNames) {
    std::string empty_name = "";
    Category c1(0, empty_name);
    EXPECT_EQ(c1.GetId(), 0);
    EXPECT_TRUE(c1.GetName().empty());

    std::string long_name = "Очень длинное название категории вопросов для викторины";
    Category c2(9999, long_name);
    EXPECT_EQ(c2.GetId(), 9999);
    EXPECT_EQ(c2.GetName(), "Очень длинное название категории вопросов для викторины");
}

TEST(CategoryTest, AddAndGetQuestions) {
    std::string cat_name = "Животные";
    Category cat(5, cat_name);
    EXPECT_TRUE(cat.GetQuestions().empty());

    std::string q_text = "Хвостатый зверь?";
    std::string diff = "Легко";
    std::string ans = "Кот";
    std::vector<std::string> alts = { "кот", "кошка" };

    auto q1 = std::make_shared<StandardQuestion>(101, 5, 100, q_text, diff, ans, alts);
    auto q2 = std::make_shared<StandardQuestion>(102, 5, 200, q_text, diff, ans, alts);

    cat.AddQuestion(q1);
    cat.AddQuestion(q2);

    ASSERT_EQ(cat.GetQuestions().size(), 2);
    EXPECT_EQ(cat.GetQuestions()[0]->GetId(), 101);
    EXPECT_EQ(cat.GetQuestions()[1]->GetId(), 102);
}

TEST(DataManagerTest, AddAndRetrieveMultiple) {
    DataManager<Category> manager;

    std::string n1 = "Кино";
    std::string n2 = "Музыка";
    std::string n3 = "Наука";

    manager.Add(std::make_shared<Category>(1, n1));
    manager.Add(std::make_shared<Category>(2, n2));
    manager.Add(std::make_shared<Category>(3, n3));

    EXPECT_EQ(manager.GetAll().size(), 3);
    EXPECT_EQ(manager.GetById(1)->GetName(), "Кино");
    EXPECT_EQ(manager.GetById(2)->GetName(), "Музыка");
    EXPECT_EQ(manager.GetById(3)->GetName(), "Наука");
}

TEST(DataManagerTest, NonExistingIdThrowsException) {
    DataManager<Category> manager;
    std::string n = "Тест";
    manager.Add(std::make_shared<Category>(5, n));

    EXPECT_THROW(manager.GetById(999), std::exception);
    EXPECT_THROW(manager.GetById(0), std::exception);
    EXPECT_THROW(manager.GetById(-1), std::exception);
}

TEST(DataManagerTest, ClearCollection) {
    DataManager<Category> manager;
    std::string name = "Спорт";
    manager.Add(std::make_shared<Category>(1, name));
    EXPECT_EQ(manager.GetAll().size(), 1);

    manager.Clear();
    EXPECT_TRUE(manager.GetAll().empty());
}

TEST(StandardQuestionTest, CorrectAndIncorrectAnswers) {
    std::string text = "Столица Беларуси?";
    std::string diff = "Легко";
    std::string primary = "Минск";
    std::vector<std::string> alts = { "Минск", "минск", "г. Минск", "город Минск" };

    StandardQuestion q(10, 1, 100, text, diff, primary, alts);

    EXPECT_EQ(q.GetId(), 10);
    EXPECT_EQ(q.GetCategoryId(), 1);
    EXPECT_EQ(q.GetCost(), 100);
    EXPECT_EQ(q.GetText(), "Столица Беларуси?");
    EXPECT_EQ(q.GetPrimaryAnswer(), "Минск");

    EXPECT_TRUE(q.CheckAnswer("Минск"));
    EXPECT_TRUE(q.CheckAnswer("минск"));
    EXPECT_TRUE(q.CheckAnswer("г. Минск"));
    EXPECT_TRUE(q.CheckAnswer("город Минск"));

    EXPECT_FALSE(q.CheckAnswer("Брест"));
    EXPECT_FALSE(q.CheckAnswer("Гомель"));
    EXPECT_FALSE(q.CheckAnswer(""));
}

TEST(StandardQuestionTest, EmptyAlternativesVector) {
    std::string text = "2 + 2 = ?";
    std::string diff = "Легко";
    std::string primary = "4";
    std::vector<std::string> empty_alts;

    StandardQuestion q(20, 1, 200, text, diff, primary, empty_alts);
    EXPECT_TRUE(q.CheckAnswer("4"));
    EXPECT_FALSE(q.CheckAnswer("5"));
}

TEST(ImageQuestionTest, ImagePathAndAnswerCheck) {
    std::string text = "Назовите эту пирамиду";
    std::string diff = "Средне";
    std::string primary = "Хеопса";
    std::vector<std::string> alts = { "Хеопса", "хеопса", "пирамида Хеопса" };
    std::string img_path = "assets/images/cheops_pyramid.jpg";

    ImageQuestion img_q(30, 2, 300, text, diff, primary, alts, img_path);

    EXPECT_EQ(img_q.GetImagePath(), img_path);
    EXPECT_TRUE(img_q.CheckAnswer("Хеопса"));
    EXPECT_TRUE(img_q.CheckAnswer("пирамида Хеопса"));
    EXPECT_FALSE(img_q.CheckAnswer("Сфинкс"));
}

TEST(ImageQuestionTest, PolymorphismViaBasePointer) {
    std::string text = "Чудо света";
    std::string diff = "Сложно";
    std::string primary = "Маяк";
    std::vector<std::string> alts = { "Маяк", "маяк" };
    std::string img_path = "assets/images/lighthouse.jpg";

    std::shared_ptr<IQuestion> question_ptr =
        std::make_shared<ImageQuestion>(31, 2, 500, text, diff, primary, alts, img_path);

    EXPECT_EQ(question_ptr->GetCost(), 500);
    EXPECT_EQ(question_ptr->GetCategoryId(), 2);
    EXPECT_TRUE(question_ptr->CheckAnswer("маяк"));

    auto casted = std::dynamic_pointer_cast<ImageQuestion>(question_ptr);
    ASSERT_NE(casted, nullptr);
    EXPECT_EQ(casted->GetImagePath(), img_path);
}

TEST(AudioQuestionTest, AudioPathAndAnswerCheck) {
    std::string text = "Назовите песню";
    std::string diff = "Легко";
    std::string primary = "Комарово";
    std::vector<std::string> alts = { "Комарово", "комарово", "На недельку до второго" };
    std::string aud_path = "assets/audio/komarovo.mp3";

    AudioQuestion aud_q(40, 3, 200, text, diff, primary, alts, aud_path);

    EXPECT_EQ(aud_q.GetAudioPath(), aud_path);
    EXPECT_TRUE(aud_q.CheckAnswer("Комарово"));
    EXPECT_TRUE(aud_q.CheckAnswer("На недельку до второго"));
    EXPECT_FALSE(aud_q.CheckAnswer("Катюша"));
}

TEST(AudioQuestionTest, PolymorphismDynamicCast) {
    std::string text = "Слушаем трек";
    std::string diff = "Сложно";
    std::string primary = "Голубой вагон";
    std::vector<std::string> alts = { "Голубой вагон", "голубой вагон" };
    std::string aud_path = "assets/audio/goluboy_vagon.mp3";

    std::shared_ptr<IQuestion> question_ptr =
        std::make_shared<AudioQuestion>(41, 3, 500, text, diff, primary, alts, aud_path);

    auto aud_cast = std::dynamic_pointer_cast<AudioQuestion>(question_ptr);
    ASSERT_NE(aud_cast, nullptr);
    EXPECT_EQ(aud_cast->GetAudioPath(), aud_path);

    auto img_cast = std::dynamic_pointer_cast<ImageQuestion>(question_ptr);
    EXPECT_EQ(img_cast, nullptr);
}

TEST(GameSessionTest, InitialState) {
    GameSession session;
    EXPECT_EQ(session.GetScore(), 0);
    EXPECT_FALSE(session.IsQuestionPlayed(1));
    EXPECT_FALSE(session.IsQuestionPlayed(100));
}

TEST(GameSessionTest, ComplexScoringSequence) {
    GameSession session;

    session.ProcessResult(200, true, 1);
    EXPECT_EQ(session.GetScore(), 200);
    EXPECT_TRUE(session.IsQuestionPlayed(1));

    session.ProcessResult(300, false, 2);
    EXPECT_EQ(session.GetScore(), -100);
    EXPECT_TRUE(session.IsQuestionPlayed(2));

    session.ProcessResult(0, false, 3);
    EXPECT_EQ(session.GetScore(), -100);
    EXPECT_TRUE(session.IsQuestionPlayed(3));

    session.ProcessResult(500, true, 4);
    EXPECT_EQ(session.GetScore(), 400);
    EXPECT_TRUE(session.IsQuestionPlayed(4));
}

TEST(GameSessionTest, CountersAccuracyAndNegativePoints) {
    GameSession session;
    EXPECT_EQ(session.GetCorrectCount(), 0);
    EXPECT_EQ(session.GetIncorrectCount(), 0);

    session.ProcessResult(100, true, 1);
    session.ProcessResult(200, false, 2);
    session.ProcessResult(300, true, 3);
    session.ProcessResult(0, false, 4);
    session.ProcessResult(-50, false, 5);

    EXPECT_EQ(session.GetCorrectCount(), 2);
    EXPECT_EQ(session.GetIncorrectCount(), 1);
}

TEST(DatabaseTest, FullInitializationAndSeeding) {
    std::string test_db = "test_game_full.db";
    if (fs::exists(test_db)) {
        fs::remove(test_db);
    }

    {
        Database db(test_db);
        EXPECT_NO_THROW(db.Initialize());

        DataManager<Category> cat_mgr;
        DataManager<IQuestion> q_mgr;

        EXPECT_NO_THROW(db.LoadCategories(cat_mgr));
        EXPECT_NO_THROW(db.LoadQuestions(q_mgr));

        auto categories = cat_mgr.GetAll();
        auto questions = q_mgr.GetAll();

        EXPECT_GE(categories.size(), 2);
        EXPECT_GE(questions.size(), 10);

        int image_count = 0;
        int audio_count = 0;
        for (const auto& q : questions) {
            if (std::dynamic_pointer_cast<ImageQuestion>(q)) image_count++;
            if (std::dynamic_pointer_cast<AudioQuestion>(q)) audio_count++;
        }

        EXPECT_GE(image_count, 5);
        EXPECT_GE(audio_count, 5);
    }

    if (fs::exists(test_db)) {
        fs::remove(test_db);
    }
}

TEST(DatabaseTest, DoubleInitializationDoesNotCrash) {
    std::string test_db = "test_game_idempotent.db";
    if (fs::exists(test_db)) fs::remove(test_db);

    {
        Database db(test_db);
        EXPECT_NO_THROW(db.Initialize());
        EXPECT_NO_THROW(db.Initialize());
    }

    if (fs::exists(test_db)) fs::remove(test_db);
}

TEST(DatabaseHelperTest, SplitStringTrimmingAndEmpty) {
    std::string sample = "Москва.|Минск|Киев.";
    auto result = SplitString(sample, '|');
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "Москва");
    EXPECT_EQ(result[1], "Минск");
    EXPECT_EQ(result[2], "Киев");

    auto empty_res = SplitString("", '|');
    EXPECT_TRUE(empty_res.empty());
}

TEST(DatabaseTest, InvalidDatabasePathThrows) {
    EXPECT_THROW({
        Database bad_db("."); // A directory is never a valid SQLite database file.
        bad_db.Initialize();
        }, std::runtime_error);
}

TEST(GameClassTest, FullWorkflow) {
    Game game;
    game.SetMode(GameMode::Training);
    game.SetMode(GameMode::Classic);
    EXPECT_EQ(game.GetSession().GetScore(), 0);

    std::string c_name = "Космос";
    auto cat = std::make_shared<Category>(10, c_name);

    std::string q_text = "Первый космонавт?";
    std::string diff = "Легко";
    std::string ans = "Гагарин";
    std::vector<std::string> alts = { "Гагарин", "гагарин", "Юрий Гагарин" };

    auto q = std::make_shared<StandardQuestion>(201, 10, 500, q_text, diff, ans, alts);
    cat->AddQuestion(q);

    std::vector<std::shared_ptr<Category>> board = { cat };
    EXPECT_NO_THROW(game.GenerateBoard(board));

    bool correct_res = game.AnswerQuestion(q, "Гагарин");
    EXPECT_TRUE(correct_res);
    EXPECT_EQ(game.GetSession().GetScore(), 500);
    EXPECT_TRUE(game.GetSession().IsQuestionPlayed(201));

    auto q_wrong = std::make_shared<StandardQuestion>(202, 10, 200, q_text, diff, ans, alts);
    bool wrong_res = game.AnswerQuestion(q_wrong, "Титов");
    EXPECT_FALSE(wrong_res);
    EXPECT_EQ(game.GetSession().GetScore(), 300);
}

TEST(IQuestionBaseTest, BaseCheckAnswerAndGetters) {
    std::string text = "Базовый вопрос";
    std::string diff = "Средне";
    std::string ans = "Базовый ответ";

    IQuestion base_q(999, 7, 400, text, diff, ans);
    EXPECT_EQ(base_q.GetId(), 999);
    EXPECT_EQ(base_q.GetCategoryId(), 7);
    EXPECT_EQ(base_q.GetCost(), 400);
    EXPECT_EQ(base_q.GetText(), "Базовый вопрос");
    EXPECT_EQ(base_q.GetPrimaryAnswer(), "Базовый ответ");
    EXPECT_FALSE(base_q.CheckAnswer("Любой ответ"));
}

TEST(AssetsWorkTest, PathResolvingVariants) {
    std::string fake = "dummy_folder/no_file.jpg";
    EXPECT_EQ(ResolveMediaPath(fake), fake);

    std::string dummy_file = "test_temp_asset.txt";
    std::ofstream out(dummy_file);
    out << "hello";
    out.close();

    std::string resolved = ResolveMediaPath(dummy_file);
    EXPECT_FALSE(resolved.empty());
    EXPECT_TRUE(fs::exists(resolved));

    fs::remove(dummy_file);
}

TEST(AssetsWorkTest, StopAudioWhenNotPlaying) {
    EXPECT_NO_THROW(StopAudioBackground());
}

TEST(AssetsWorkTest, AudioPlaybackTriggers) {
    EXPECT_NO_THROW(PlayAudioBackground("assets/audio/komarovo.mp3"));
    EXPECT_NO_THROW(StopAudioBackground());

    EXPECT_NO_THROW(PlayAudioBackground("invalid_audio_file_path.mp3"));
}

TEST(AssetsWorkTest, ViewerAndResolverEdgeCases) {
    EXPECT_NO_THROW(OpenImageInViewer("non_existent_folder/absent_image.jpg"));

    std::string non_existent = "random_unreal_path/file.png";
    EXPECT_EQ(ResolveMediaPath(non_existent), non_existent);
}

namespace {

std::shared_ptr<IQuestion> MakeQuestion(int id, int cat_id, int cost, const std::string& answer) {
    std::string text = "Вопрос " + std::to_string(id);
    std::string diff = "Легко";
    std::string ans = answer;
    std::vector<std::string> alts = { answer };
    return std::make_shared<StandardQuestion>(id, cat_id, cost, text, diff, ans, alts);
}

std::vector<std::shared_ptr<IQuestion>> MakePool(int cat_count) {
    std::vector<std::shared_ptr<IQuestion>> pool;
    int id = 1;
    for (int cat = 1; cat <= cat_count; cat++) {
        for (int cost = 100; cost <= 500; cost += 100) {
            pool.push_back(MakeQuestion(id, cat, cost, "ответ" + std::to_string(id)));
            id++;
        }
    }
    return pool;
}

DataManager<Category> MakeCategories() {
    DataManager<Category> manager;
    std::string n1 = "История";
    std::string n2 = "Кино";
    std::string n3 = "Наука";
    manager.Add(std::make_shared<Category>(1, n1));
    manager.Add(std::make_shared<Category>(2, n2));
    manager.Add(std::make_shared<Category>(3, n3));
    return manager;
}

}

TEST(StatisticsTest, CostBandBoundaries) {
    EXPECT_EQ(GetCostBand(100), CostBand::Easy);
    EXPECT_EQ(GetCostBand(200), CostBand::Easy);
    EXPECT_EQ(GetCostBand(300), CostBand::Medium);
    EXPECT_EQ(GetCostBand(400), CostBand::Hard);
    EXPECT_EQ(GetCostBand(500), CostBand::Hard);

    EXPECT_EQ(GetCostBandLabel(CostBand::Easy), "100–200 баллов");
    EXPECT_EQ(GetCostBandLabel(CostBand::Medium), "300 баллов");
    EXPECT_EQ(GetCostBandLabel(CostBand::Hard), "400–500 баллов");
}

TEST(StatisticsTest, PercentRounding) {
    StatEntry empty;
    EXPECT_EQ(empty.GetPercent(), 0);

    StatEntry e1{ 4, 5 };
    EXPECT_EQ(e1.GetPercent(), 80);

    StatEntry e2{ 2, 3 };
    EXPECT_EQ(e2.GetPercent(), 67);

    StatEntry e3{ 1, 3 };
    EXPECT_EQ(e3.GetPercent(), 33);

    StatEntry e4{ 5, 5 };
    EXPECT_EQ(e4.GetPercent(), 100);
}

TEST(StatisticsTest, AggregatesByCategoryAndBand) {
    Statistics stats;
    EXPECT_TRUE(stats.IsEmpty());

    stats.AddResult(1, 100, true);
    stats.AddResult(1, 200, false);
    stats.AddResult(2, 300, true);
    stats.AddResult(2, 500, false);
    stats.AddResult(2, 400, true);

    EXPECT_FALSE(stats.IsEmpty());
    EXPECT_EQ(stats.GetTotal().total, 5);
    EXPECT_EQ(stats.GetTotal().correct, 3);

    const auto& cats = stats.GetCategoryStats();
    ASSERT_EQ(cats.size(), 2);
    EXPECT_EQ(cats.at(1).correct, 1);
    EXPECT_EQ(cats.at(1).total, 2);
    EXPECT_EQ(cats.at(2).correct, 2);
    EXPECT_EQ(cats.at(2).total, 3);

    const auto& bands = stats.GetCostBandStats();
    ASSERT_EQ(bands.size(), 3);
    EXPECT_EQ(bands.at(CostBand::Easy).total, 2);
    EXPECT_EQ(bands.at(CostBand::Easy).correct, 1);
    EXPECT_EQ(bands.at(CostBand::Medium).correct, 1);
    EXPECT_EQ(bands.at(CostBand::Hard).total, 2);
    EXPECT_EQ(bands.at(CostBand::Hard).GetPercent(), 50);
}

TEST(TrainingHelpersTest, ParseIdList) {
    EXPECT_EQ(ParseIdList("1 5, 12"), (std::vector<int>{ 1, 5, 12 }));
    EXPECT_EQ(ParseIdList("7"), (std::vector<int>{ 7 }));
    EXPECT_EQ(ParseIdList("  3;4;; 0 "), (std::vector<int>{ 3, 4, 0 }));
    EXPECT_TRUE(ParseIdList("").empty());
    EXPECT_TRUE(ParseIdList("история").empty());
    EXPECT_TRUE(ParseIdList("12345678901234").empty());
}

TEST(TrainingHelpersTest, FilterByCategoriesAndCost) {
    auto pool = MakePool(3);

    TrainingSettings all;
    EXPECT_EQ(FilterQuestions(pool, all).size(), 15);

    TrainingSettings settings;
    settings.category_ids = { 1, 3 };
    settings.min_cost = 200;
    settings.max_cost = 400;
    auto filtered = FilterQuestions(pool, settings);
    ASSERT_EQ(filtered.size(), 6);
    for (const auto& q : filtered) {
        EXPECT_NE(q->GetCategoryId(), 2);
        EXPECT_GE(q->GetCost(), 200);
        EXPECT_LE(q->GetCost(), 400);
    }
}

TEST(TrainingSessionTest, GenerateValidatesSettings) {
    auto pool = MakePool(2);
    TrainingSession session;

    TrainingSettings bad_range;
    bad_range.min_cost = 500;
    bad_range.max_cost = 100;
    EXPECT_THROW(session.Generate(pool, bad_range), std::invalid_argument);

    TrainingSettings bad_count;
    bad_count.question_count = 0;
    EXPECT_THROW(session.Generate(pool, bad_count), std::invalid_argument);

    TrainingSettings nothing;
    nothing.category_ids = { 42 };
    EXPECT_THROW(session.Generate(pool, nothing), std::runtime_error);
}

TEST(TrainingSessionTest, GenerateRespectsSettingsAndBalancesTopics) {
    auto pool = MakePool(3);

    TrainingSettings settings;
    settings.category_ids = { 1, 2, 3 };
    settings.min_cost = 100;
    settings.max_cost = 500;
    settings.question_count = 6;

    TrainingSession session;
    session.Generate(pool, settings);
    ASSERT_EQ(session.GetQuestionCount(), 6);

    std::map<int, int> per_category;
    std::vector<int> ids;
    while (!session.IsFinished()) {
        auto q = session.GetCurrentQuestion();
        per_category[q->GetCategoryId()]++;
        ids.push_back(q->GetId());
        session.SubmitAnswer("");
    }
    EXPECT_EQ(per_category[1], 2);
    EXPECT_EQ(per_category[2], 2);
    EXPECT_EQ(per_category[3], 2);
    std::sort(ids.begin(), ids.end());
    EXPECT_EQ(std::unique(ids.begin(), ids.end()), ids.end());
}

TEST(TrainingSessionTest, CountIsLimitedByAvailableQuestions) {
    auto pool = MakePool(2);

    TrainingSettings settings;
    settings.category_ids = { 2 };
    settings.min_cost = 400;
    settings.max_cost = 500;
    settings.question_count = 50;

    TrainingSession session;
    session.Generate(pool, settings);
    EXPECT_EQ(session.GetQuestionCount(), 2);
}

TEST(TrainingSessionTest, AnswersHistoryAndStatistics) {
    std::vector<std::shared_ptr<IQuestion>> questions = {
        MakeQuestion(1, 1, 100, "Гагарин"),
        MakeQuestion(2, 1, 300, "Титов"),
        MakeQuestion(3, 2, 500, "Королёв")
    };

    TrainingSession session;
    session.LoadQuestions(questions);
    EXPECT_EQ(session.GetQuestionCount(), 3);
    EXPECT_FALSE(session.IsFinished());
    EXPECT_EQ(session.GetCurrentNumber(), 1);

    EXPECT_TRUE(session.SubmitAnswer("  Гагарин \t"));
    EXPECT_EQ(session.GetCurrentNumber(), 2);
    EXPECT_FALSE(session.SubmitAnswer("Леонов"));
    EXPECT_FALSE(session.SubmitAnswer(""));

    EXPECT_TRUE(session.IsFinished());
    EXPECT_THROW(session.GetCurrentQuestion(), std::out_of_range);
    EXPECT_THROW(session.SubmitAnswer("Королёв"), std::out_of_range);

    const auto& history = session.GetHistory();
    ASSERT_EQ(history.size(), 3);
    EXPECT_EQ(history[0].user_answer, "Гагарин");
    EXPECT_TRUE(history[0].is_correct);
    EXPECT_EQ(history[1].user_answer, "Леонов");
    EXPECT_TRUE(history[2].user_answer.empty());

    const Statistics& stats = session.GetStatistics();
    EXPECT_EQ(stats.GetTotal().correct, 1);
    EXPECT_EQ(stats.GetTotal().total, 3);
    EXPECT_EQ(stats.GetCategoryStats().at(1).GetPercent(), 50);
    EXPECT_EQ(stats.GetCategoryStats().at(2).GetPercent(), 0);
}

TEST(TrainingSessionTest, RepeatMistakesBuildsMiniTraining) {
    std::vector<std::shared_ptr<IQuestion>> questions = {
        MakeQuestion(1, 1, 100, "а"),
        MakeQuestion(2, 1, 200, "б"),
        MakeQuestion(3, 2, 300, "в"),
        MakeQuestion(4, 2, 400, "г")
    };

    TrainingSession session;
    session.LoadQuestions(questions);
    session.SubmitAnswer("а");
    session.SubmitAnswer("неверно");
    session.SubmitAnswer("в");
    session.SubmitAnswer("");

    auto mistakes = session.GetMistakes();
    ASSERT_EQ(mistakes.size(), 2);
    EXPECT_EQ(mistakes[0]->GetId(), 2);
    EXPECT_EQ(mistakes[1]->GetId(), 4);

    TrainingSession review = session.CreateMistakesReview();
    EXPECT_EQ(review.GetQuestionCount(), 2);
    EXPECT_TRUE(review.GetStatistics().IsEmpty());
    EXPECT_TRUE(review.GetHistory().empty());

    EXPECT_TRUE(review.SubmitAnswer("б"));
    EXPECT_FALSE(review.SubmitAnswer("д"));
    EXPECT_TRUE(review.IsFinished());

    TrainingSession second_review = review.CreateMistakesReview();
    ASSERT_EQ(second_review.GetQuestionCount(), 1);
    EXPECT_EQ(second_review.GetCurrentQuestion()->GetId(), 4);
    EXPECT_TRUE(second_review.SubmitAnswer("г"));
    EXPECT_TRUE(second_review.GetMistakes().empty());

    EXPECT_EQ(session.GetMistakes().size(), 2);
}

TEST(RecommendationTest, ExampleFromTaskDescription) {
    Statistics stats;
    stats.AddResult(1, 100, true);
    stats.AddResult(1, 200, true);
    stats.AddResult(1, 300, true);
    stats.AddResult(1, 400, true);
    stats.AddResult(1, 500, false);
    stats.AddResult(2, 100, true);
    stats.AddResult(2, 200, true);
    stats.AddResult(2, 300, false);
    stats.AddResult(2, 400, false);
    stats.AddResult(2, 500, false);
    stats.AddResult(3, 100, true);
    stats.AddResult(3, 200, true);
    stats.AddResult(3, 300, true);
    stats.AddResult(3, 400, false);
    stats.AddResult(3, 500, false);

    EXPECT_EQ(stats.GetCategoryStats().at(1).GetPercent(), 80);
    EXPECT_EQ(stats.GetCategoryStats().at(2).GetPercent(), 40);
    EXPECT_EQ(stats.GetCategoryStats().at(3).GetPercent(), 60);

    RecommendationSystem recommender;
    EXPECT_EQ(recommender.GetThreshold(), 60);

    Recommendation rec = recommender.Analyze(stats);
    EXPECT_FALSE(rec.IsEmpty());
    EXPECT_EQ(rec.weak_category_ids, (std::vector<int>{ 2 }));
    EXPECT_EQ(rec.weak_cost_bands, (std::vector<CostBand>{ CostBand::Hard }));

    auto categories = MakeCategories();
    EXPECT_EQ(recommender.BuildText(rec, categories),
        "Рекомендуется потренировать тему «Кино» и вопросы стоимостью 400–500 баллов.");
}

TEST(RecommendationTest, SeveralWeakSpotsSortedFromWeakest) {
    Statistics stats;
    stats.AddResult(1, 100, true);
    stats.AddResult(1, 300, false);
    stats.AddResult(2, 300, false);
    stats.AddResult(2, 500, false);
    stats.AddResult(3, 100, true);

    RecommendationSystem recommender;
    Recommendation rec = recommender.Analyze(stats);
    EXPECT_EQ(rec.weak_category_ids, (std::vector<int>{ 2, 1 }));
    EXPECT_EQ(rec.weak_cost_bands, (std::vector<CostBand>{ CostBand::Medium, CostBand::Hard }));

    auto categories = MakeCategories();
    EXPECT_EQ(recommender.BuildText(rec, categories),
        "Рекомендуется потренировать темы «Кино» и «История», а также вопросы стоимостью 300 баллов и 400–500 баллов.");
}

TEST(RecommendationTest, OnlyTopicsOrOnlyBands) {
    auto categories = MakeCategories();
    RecommendationSystem recommender;

    Recommendation topics_only;
    topics_only.weak_category_ids = { 1, 2, 3 };
    EXPECT_EQ(recommender.BuildText(topics_only, categories),
        "Рекомендуется потренировать темы «История», «Кино» и «Наука».");

    Recommendation bands_only;
    bands_only.weak_cost_bands = { CostBand::Easy };
    EXPECT_EQ(recommender.BuildText(bands_only, categories),
        "Рекомендуется потренировать вопросы стоимостью 100–200 баллов.");

    Recommendation unknown_topic;
    unknown_topic.weak_category_ids = { 99 };
    EXPECT_EQ(recommender.BuildText(unknown_topic, categories),
        "Рекомендуется потренировать тему №99.");
}

TEST(RecommendationTest, NoWeakSpotsAndCustomThreshold) {
    Statistics stats;
    stats.AddResult(1, 100, true);
    stats.AddResult(1, 400, true);
    stats.AddResult(2, 300, true);
    stats.AddResult(2, 500, false);

    RecommendationSystem lenient(50);
    Recommendation rec = lenient.Analyze(stats);
    EXPECT_TRUE(rec.IsEmpty());

    auto categories = MakeCategories();
    std::string text = lenient.BuildText(rec, categories);
    EXPECT_EQ(text.rfind("Отличный результат!", 0), 0);
    EXPECT_NE(text.find("50%"), std::string::npos);

    RecommendationSystem strict(90);
    Recommendation strict_rec = strict.Analyze(stats);
    EXPECT_EQ(strict_rec.weak_category_ids, (std::vector<int>{ 2 }));

    Statistics empty_stats;
    EXPECT_TRUE(strict.Analyze(empty_stats).IsEmpty());
}
