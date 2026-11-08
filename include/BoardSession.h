#pragma once
#include "Category.h"
#include "CatQuestion.h"
#include <random>
#include <set>

struct BoardRow {
    int categoryId;
    std::string name;
    std::vector<std::shared_ptr<IQuestion>> questions;
};

// Owns the theme deck for one session; a theme can occur on only one board.
class BoardSession {
public:
    explicit BoardSession(unsigned seed = std::random_device{}()) : random(seed) {}
    void Reset(const std::vector<std::shared_ptr<Category>>& categories,
               const std::vector<std::shared_ptr<IQuestion>>& questions);
    bool NextBoard();
    const std::vector<BoardRow>& GetRows() const { return rows; }
    int GetCatSlot() const { for (int slot : catSlots) if (!IsUsed(slot)) return slot; return -1; }
    bool IsCatSlot(int cell) const { return catSlots.count(cell) != 0; }
    const std::set<int>& GetCatSlots() const { return catSlots; }
    std::string GetCatTheme() const { return deck.empty() ? "" : deck.back().name; }
    int GetRound() const { return round; }
    bool HasNextBoard() const { return deck.size() >= 9; }
    bool IsUsed(int cell) const { return usedCells.count(cell) != 0; }
    bool IsComplete() const { return usedCells.size() == 30; }
    std::shared_ptr<IQuestion> Select(int cell, int catStake = 0);
private:
    std::mt19937 random;
    std::vector<BoardRow> deck, rows;
    std::set<int> usedCells;
    std::set<int> catSlots;
    int round = 0;
};
