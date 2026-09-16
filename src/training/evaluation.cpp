#include "bond/evaluation.hpp"
#include <charconv>
#include <cctype>
#include <string_view>

namespace bond {
namespace {
bool contains_construct(std::string_view source, std::string_view construct) {
  if (construct.empty()) return true;
  constexpr std::string_view function_prefix{"function:"};
  if (construct.starts_with(function_prefix)) {
    const auto name = construct.substr(function_prefix.size());
    std::size_t position{};
    while ((position = source.find(name, position)) != std::string_view::npos) {
      const bool left_boundary = position == 0 || !(std::isalnum(static_cast<unsigned char>(source[position - 1])) || source[position - 1] == '_');
      auto after_name = position + name.size();
      while (after_name < source.size() && std::isspace(static_cast<unsigned char>(source[after_name]))) ++after_name;
      if (left_boundary && after_name < source.size() && source[after_name] == '(') return true;
      position += name.size();
    }
    return false;
  }
  const auto separator = construct.rfind('*');
  if (separator == std::string_view::npos) return source.find(construct) != std::string_view::npos;
  int required{};
  const auto count_text = construct.substr(separator + 1);
  const auto [end, error] = std::from_chars(count_text.data(), count_text.data() + count_text.size(), required);
  if (error != std::errc{} || end != count_text.data() + count_text.size() || required < 1) return false;
  const auto token = construct.substr(0, separator); int occurrences{}; std::size_t position{};
  while ((position = source.find(token, position)) != std::string_view::npos) { ++occurrences; position += token.size(); }
  return occurrences >= required;
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
