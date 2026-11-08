#include <gtest/gtest.h>
#include "BoardSession.h"
#include "Database.h"
#include "StandardQuestion.h"
#include <set>

struct BoardFixture : testing::Test {
    DataManager<Category> categories;
    DataManager<IQuestion> questions;
    BoardSession board{42};
    void SetUp() override {
        Database db(":memory:"); db.Initialize(); db.LoadCategories(categories); db.LoadQuestions(questions);
        board.Reset(categories.GetAll(), questions.GetAll());
    }
    void Complete() {
        for (int cell = 0; cell < 30; ++cell) board.Select(cell, cell == board.GetCatSlot() ? 100 : 0);
    }
};
TEST_F(BoardFixture, SixCompleteThemesAndFivePrices) {
    ASSERT_TRUE(board.NextBoard()); ASSERT_EQ(board.GetRows().size(), 6u); EXPECT_EQ(board.GetCatSlots().size(),3u);
    std::set<int> themes;
    for (const auto& row : board.GetRows()) {
        themes.insert(row.categoryId); ASSERT_EQ(row.questions.size(), 5u);
        for (int i = 0; i < 5; ++i) EXPECT_EQ(row.questions[i]->GetCost(), (i+1)*100);
    }
    EXPECT_EQ(themes.size(), 6u);
    EXPECT_THROW(board.NextBoard(), std::logic_error);
}
TEST_F(BoardFixture, NoThemeRepeatsIncludingCatAcrossAllBoards) {
    std::set<int> themes;
    int rounds = 0;
    while (board.NextBoard()) {
        ++rounds;
        for (const auto& row : board.GetRows()) EXPECT_TRUE(themes.insert(row.categoryId).second);
        for (int i=0; i<30; ++i) {
            auto q=board.Select(i, i==board.GetCatSlot() ? 500 : 0);
            if(auto cat=std::dynamic_pointer_cast<CatQuestion>(q)) {
                EXPECT_TRUE(themes.insert(q->GetCategoryId()).second);
                EXPECT_EQ(q->GetCost(), 500);
            }
        }
        EXPECT_TRUE(board.IsComplete());
    }
    EXPECT_GE(rounds, 2);
    EXPECT_FALSE(board.HasNextBoard());
}
TEST_F(BoardFixture, InvalidAndRepeatedSelectionsDoNotConsumeQuestions) {
    EXPECT_THROW(board.Select(0), std::out_of_range);
    ASSERT_TRUE(board.NextBoard());
    EXPECT_THROW(board.Select(-1), std::out_of_range);
    EXPECT_THROW(board.Select(30), std::out_of_range);
    int cat=board.GetCatSlot(); ASSERT_GE(cat,0);
    EXPECT_THROW(board.Select(cat,300), std::invalid_argument); EXPECT_FALSE(board.IsUsed(cat));
    auto q=board.Select(cat,100); EXPECT_EQ(q->GetCost(),100);
    EXPECT_THROW(board.Select(cat,500), std::out_of_range);
}
TEST_F(BoardFixture, ResetRestartsSessionAndIncompletePoolsStopCleanly) {
    ASSERT_TRUE(board.NextBoard()); Complete();
    board.Reset(categories.GetAll(),questions.GetAll()); EXPECT_EQ(board.GetRound(),0);
    EXPECT_TRUE(board.NextBoard()); EXPECT_FALSE(board.IsUsed(0));
    board.Reset({},{}); EXPECT_FALSE(board.NextBoard()); EXPECT_EQ(board.GetCatSlot(),-1);
    auto few=categories.GetAll(); few.resize(5);
    board.Reset(few,questions.GetAll()); EXPECT_FALSE(board.NextBoard());
}
TEST_F(BoardFixture, NineThemesReserveThreeCatsAndDuplicateCategoriesAreIgnored) {
    auto six=categories.GetAll(); six.resize(9); six.push_back(six.front());
    board.Reset(six,questions.GetAll()); ASSERT_TRUE(board.NextBoard());
    EXPECT_EQ(board.GetCatSlots().size(),3u); Complete(); EXPECT_FALSE(board.NextBoard());
}
TEST(CatQuestionTest, DelegatesAliasesAndPreservesMediaSource) {
    std::string text="Question", diff="Easy", answer="Main"; std::vector<std::string> aliases{"Alias"};
    auto source=std::make_shared<StandardQuestion>(1,2,300,text,diff,answer,aliases);
    CatQuestion cat(source,500);
    EXPECT_TRUE(cat.CheckAnswer("Alias")); EXPECT_TRUE(cat.CheckAnswer("Main"));
    EXPECT_FALSE(cat.CheckAnswer("wrong")); EXPECT_EQ(cat.GetText(),source->GetText());
    EXPECT_EQ(cat.GetSource(),source); EXPECT_EQ(cat.GetCategoryId(),2); EXPECT_EQ(cat.GetCost(),500);
    EXPECT_THROW(CatQuestion(source,200),std::invalid_argument);
    EXPECT_THROW(CatQuestion(nullptr,100),std::invalid_argument);
}
