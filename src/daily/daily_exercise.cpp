#include "bond/daily_exercise.hpp"
namespace bond { DailyExercise generate_daily_exercise(std::uint64_t seed) { return {seed, static_cast<int>(seed % 5U) + 1, "daily.harvest"}; } }
