#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace bond {
struct GridPosition { int x{}; int y{}; friend bool operator==(GridPosition, GridPosition) = default; };
enum class Cell { empty, crop, wall, depot };
enum class Command { move_north, move_south, move_west, move_east, harvest, deposit };
struct SimulationSnapshot { GridPosition robot; int inventory{}; int harvested{}; bool complete{}; int actions{}; int remaining_crops{}; };
struct Lesson {
  int id{};
  int module{};
  std::string key;
  std::string title_key;
  std::string requirement_key;
  int target_harvest{};
  std::string required_construct;
  std::string feedback_key;
};
struct Progress { int schema_version{1}; std::vector<int> completed_lessons; };
struct DailyExercise { std::uint64_t seed{}; int target_harvest{}; std::string title_key; };
}
