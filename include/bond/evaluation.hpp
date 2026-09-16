#pragma once
#include "bond/domain.hpp"
#include <string>
#include <vector>
namespace bond {
struct EvaluationResult { bool passed{}; int score{}; std::vector<std::string> feedback_keys; };
class Evaluator { public: virtual ~Evaluator() = default; virtual EvaluationResult evaluate(const Lesson&, const SimulationSnapshot&) const = 0; };
}
