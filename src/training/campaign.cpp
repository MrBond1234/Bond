#include "bond/training.hpp"
#include <charconv>
#include <fstream>
#include <sstream>
namespace bond {
namespace {
bool parse_int(const std::string& text, int& value) {
  const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
  return error == std::errc{} && end == text.data() + text.size();
}
}
bool Campaign::load(const std::filesystem::path& file) {
  std::ifstream input(file);
  if (!input) return false;
  std::string line;
  if (!std::getline(input, line) || line != "id|module|key|title_key|requirement_key|target_harvest") return false;
  std::vector<Lesson> loaded;
  while (std::getline(input, line)) {
    if (line.empty()) continue;
    std::stringstream row(line);
    std::vector<std::string> fields;
    std::string field;
    while (std::getline(row, field, '|')) fields.push_back(std::move(field));
    if (fields.size() != 6) return false;
    Lesson lesson;
    if (!parse_int(fields[0], lesson.id) || !parse_int(fields[1], lesson.module) ||
        !parse_int(fields[5], lesson.target_harvest) || lesson.id <= 0 || lesson.module <= 0 ||
        lesson.target_harvest <= 0 || fields[2].empty() || fields[3].empty() || fields[4].empty()) return false;
    lesson.key = std::move(fields[2]);
    lesson.title_key = std::move(fields[3]);
    lesson.requirement_key = std::move(fields[4]);
    for (const auto& existing : loaded) if (existing.id == lesson.id) return false;
    loaded.push_back(std::move(lesson));
  }
  if (loaded.empty()) return false;
  lessons_ = std::move(loaded);
  return true;
}
std::optional<Lesson> Campaign::find(int id) const { for(const auto& lesson: lessons_) if(lesson.id==id) return lesson; return std::nullopt; }
}
