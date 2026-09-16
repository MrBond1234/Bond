#pragma once
#include "bond/domain.hpp"
#include <string>
#include <vector>
namespace bond {
struct EvaluationResult { bool passed{}; int score{}; std::vector<std::string> feedback_keys; };
class Evaluator { public: virtual ~Evaluator() = default; virtual EvaluationResult evaluate(const Lesson&, const SimulationSnapshot&) const = 0; };
// A deliberately small source-level check used before simulation scoring. It is
// not a compiler and never executes learner code.
class LessonEvaluator final : public Evaluator {
public:
  [[nodiscard]] EvaluationResult evaluate(const Lesson&, const SimulationSnapshot&) const override;
  [[nodiscard]] EvaluationResult evaluate_source(const Lesson&, std::string_view source,
                                                 const SimulationSnapshot&) const;
};
}
