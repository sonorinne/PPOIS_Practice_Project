#include "BoardSession.h"
#include <algorithm>
#include <stdexcept>

void BoardSession::Reset(const std::vector<std::shared_ptr<Category>>& categories,
                         const std::vector<std::shared_ptr<IQuestion>>& questions) {
    deck.clear(); rows.clear(); usedCells.clear(); round = 0; catSlot = -1;
    std::set<int> ids;
    for (const auto& category : categories) {
        if (!category || !ids.insert(category->GetId()).second) continue;
        BoardRow row{category->GetId(), category->GetName(), {}};
        for (int cost = 100; cost <= 500; cost += 100) {
            auto q = std::find_if(questions.begin(), questions.end(), [&](const auto& q) {
                return q && q->GetCategoryId() == row.categoryId && q->GetCost() == cost;
            });
            if (q != questions.end()) row.questions.push_back(*q);
        }
        if (row.questions.size() == 5) deck.push_back(row);
    }
    std::shuffle(deck.begin(), deck.end(), random);
}
bool BoardSession::NextBoard() {
    if (!rows.empty() && !IsComplete()) throw std::logic_error("Finish the current board first");
    if (!HasNextBoard()) return false;
    rows.assign(deck.end() - 6, deck.end());
    deck.erase(deck.end() - 6, deck.end());
    usedCells.clear(); ++round;
    // Reserve a fresh theme for the cat; no theme in the session repeats.
    catSlot = deck.empty() ? -1 : std::uniform_int_distribution<int>(0, 29)(random);
    return true;
}
std::shared_ptr<IQuestion> BoardSession::Select(int cell, int stake) {
    if (cell < 0 || cell >= 30 || rows.size() != 6 || IsUsed(cell))
        throw std::out_of_range("Unavailable board cell");
    std::shared_ptr<IQuestion> question;
    if (cell == catSlot) {
        if (stake != 100 && stake != 500) throw std::invalid_argument("Choose 100 or 500");
        const auto& row = deck.back();
        question = std::make_shared<CatQuestion>(row.questions[stake / 100 - 1], stake);
        deck.pop_back();
    } else {
        question = rows[cell / 5].questions[cell % 5];
    }
    usedCells.insert(cell);
    return question;
}
