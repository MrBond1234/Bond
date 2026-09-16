#include "bond/persistence.hpp"
#include <charconv>
#include <fstream>
#include <sstream>
namespace bond {
bool ProgressStore::save(const std::filesystem::path& file, const Progress& progress) { std::ofstream out(file); if(!out) return false; out << "version=" << progress.schema_version << '\n' << "completed="; for(auto id: progress.completed_lessons) out << id << ','; return static_cast<bool>(out); }
namespace {
bool parse_int(const std::string& text, int& value) {
  const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
  return error == std::errc{} && end == text.data() + text.size();
}
}
Progress ProgressStore::load(const std::filesystem::path& file) {
  Progress value;
  std::ifstream in(file);
  std::string line;
  if (!in) return value;
  while (std::getline(in, line)) {
    if (line.rfind("version=", 0) == 0) {
      int version{};
      if (parse_int(line.substr(8), version) && version > 0) value.schema_version = version;
    }
    if (line.rfind("completed=", 0) == 0) {
      std::stringstream ids(line.substr(10));
      std::string id;
      while (std::getline(ids, id, ',')) {
        int parsed{};
        if (!id.empty() && parse_int(id, parsed) && parsed > 0) value.completed_lessons.push_back(parsed);
      }
    }
  }
  return value;
}
}
