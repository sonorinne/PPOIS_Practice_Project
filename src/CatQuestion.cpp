#include "CatQuestion.h"
#include <stdexcept>
namespace {
std::shared_ptr<IQuestion> Checked(std::shared_ptr<IQuestion> q, int stake) {
    if (!q || (stake != 100 && stake != 500))
        throw std::invalid_argument("Cat question needs a source and a stake of 100 or 500");
    return q;
}
}
CatQuestion::CatQuestion(std::shared_ptr<IQuestion> q, int stake)
    : CatQuestion(Checked(q, stake), stake, q ? q->GetText() : "", q ? q->GetPrimaryAnswer() : "") {}
CatQuestion::CatQuestion(std::shared_ptr<IQuestion> q, int stake, std::string text, std::string answer, std::string difficulty)
    : IQuestion(q->GetId(), q->GetCategoryId(), stake, text, difficulty, answer), source(q) {}
bool CatQuestion::CheckAnswer(const std::string& answer) const { return source->CheckAnswer(answer); }
