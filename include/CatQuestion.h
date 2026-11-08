#pragma once
#include "IQuestion.h"
#include <memory>

// A priced special question. Answer checking remains polymorphic (including aliases).
class CatQuestion final : public IQuestion {
public:
    CatQuestion(std::shared_ptr<IQuestion> source, int stake);
    bool CheckAnswer(const std::string& answer) const override;
    const std::shared_ptr<IQuestion>& GetSource() const { return source; }
private:
    CatQuestion(std::shared_ptr<IQuestion> source, int stake, std::string text, std::string answer, std::string difficulty = "Кот в мешке");
    std::shared_ptr<IQuestion> source;
};
