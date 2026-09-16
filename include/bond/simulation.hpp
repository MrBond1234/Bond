#pragma once
#include "bond/domain.hpp"
#include <vector>
namespace bond {
class Simulation {
public:
  Simulation(int width = 6, int height = 6);
  void reset(int target_harvest);
  bool step(Command command);
  [[nodiscard]] SimulationSnapshot snapshot() const;
private:
  bool move(int dx, int dy);
  int width_, height_, target_{};
  GridPosition robot_{1, 1};
  std::vector<Cell> cells_;
  int inventory_{}, harvested_{};
  int actions_{};
};
}
