#pragma once
#include "bond/domain.hpp"
#include <filesystem>
#include <optional>
namespace bond {
class Campaign {
public:
  bool load(const std::filesystem::path& file);
  [[nodiscard]] const std::vector<Lesson>& lessons() const { return lessons_; }
  [[nodiscard]] std::optional<Lesson> find(int id) const;
private: std::vector<Lesson> lessons_;
};
}
