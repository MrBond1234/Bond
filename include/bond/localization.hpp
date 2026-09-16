#pragma once
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
namespace bond {
class Localizer {
public:
  bool load(const std::filesystem::path& file);
  [[nodiscard]] std::string text(std::string_view key) const;
private: std::unordered_map<std::string, std::string> entries_;
};
}
