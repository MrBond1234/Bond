#include "bond/localization.hpp"
#include <fstream>
#include <sstream>

namespace bond {
namespace {
bool load_entries(const std::filesystem::path& file, std::unordered_map<std::string, std::string>& destination) {
  std::ifstream input(file);
  if (!input) return false;
  std::string line;
  if (!std::getline(input, line) || line != "key|text") return false;
  std::unordered_map<std::string, std::string> loaded;
  while (std::getline(input, line)) {
    if (line.empty()) continue;
    const auto separator = line.find('|');
    if (separator == std::string::npos || separator == 0 || separator == line.size() - 1) return false;
    auto [entry, inserted] = loaded.emplace(line.substr(0, separator), line.substr(separator + 1));
    if (!inserted) return false;
  }
  destination = std::move(loaded);
  return true;
}
}
bool Localizer::load(const std::filesystem::path& file) { return load_entries(file, entries_); }
bool Localizer::load_fallback(const std::filesystem::path& file) { return load_entries(file, fallback_entries_); }

std::string Localizer::text(std::string_view key) const {
  const auto found = entries_.find(std::string(key));
  if (found != entries_.end()) return found->second;
  const auto fallback = fallback_entries_.find(std::string(key));
  return fallback == fallback_entries_.end() ? std::string(key) : fallback->second;
}
}
