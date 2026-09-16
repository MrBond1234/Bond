#include "bond/evaluation.hpp"
#include <string_view>

namespace bond {
namespace {
bool contains_construct(std::string_view source, std::string_view construct) {
  return construct.empty() || source.find(construct) != std::string_view::npos;
}
}

EvaluationResult LessonEvaluator::evaluate(const Lesson& lesson, const SimulationSnapshot& snapshot) const {
  return evaluate_source(lesson, {}, snapshot);
}

EvaluationResult LessonEvaluator::evaluate_source(const Lesson& lesson, std::string_view source,
                                                  const SimulationSnapshot& snapshot) const {
  EvaluationResult result;
  const bool construct_ok = contains_construct(source, lesson.required_construct);
  const bool simulation_ok = snapshot.complete && snapshot.harvested >= lesson.target_harvest;
  result.passed = construct_ok && simulation_ok;
  result.score = result.passed ? 100 : (construct_ok ? 50 : 0);
  if (!construct_ok && !lesson.feedback_key.empty()) result.feedback_keys.push_back(lesson.feedback_key);
  if (!simulation_ok) result.feedback_keys.push_back("feedback.simulation.incomplete");
  return result;
}
}
