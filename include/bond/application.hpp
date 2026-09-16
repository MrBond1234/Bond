#pragma once
#include "bond/simulation.hpp"
#include "bond/training.hpp"
namespace bond {
class Application {
public:
  bool initialize(const std::filesystem::path& lesson_file);
  bool start_lesson(int id);
  bool step_simulation(Command command);
  [[nodiscard]] const Campaign& campaign() const { return campaign_; }
  [[nodiscard]] const Simulation& simulation() const { return simulation_; }
private: Campaign campaign_; Simulation simulation_; };
}
