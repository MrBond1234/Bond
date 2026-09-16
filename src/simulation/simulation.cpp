#include "bond/simulation.hpp"
#include <algorithm>
namespace bond {
Simulation::Simulation(int width, int height) : width_(width), height_(height), cells_(width * height, Cell::empty) { reset(1); }
void Simulation::reset(int target_harvest) { target_ = target_harvest; robot_ = {1, 1}; inventory_ = harvested_ = 0; std::fill(cells_.begin(), cells_.end(), Cell::empty); cells_[0] = Cell::depot; for (int i = 0; i < target_ && i + 2 < width_ * height_; ++i) cells_[i + 2] = Cell::crop; }
bool Simulation::move(int dx, int dy) { GridPosition next{robot_.x + dx, robot_.y + dy}; if (next.x < 0 || next.y < 0 || next.x >= width_ || next.y >= height_ || cells_[next.y * width_ + next.x] == Cell::wall) return false; robot_ = next; return true; }
bool Simulation::step(Command c) { switch (c) { case Command::move_north: return move(0,-1); case Command::move_south: return move(0,1); case Command::move_west: return move(-1,0); case Command::move_east: return move(1,0); case Command::harvest: { auto& cell=cells_[robot_.y*width_+robot_.x]; if(cell!=Cell::crop) return false; cell=Cell::empty; ++inventory_; return true; } case Command::deposit: if(cells_[robot_.y*width_+robot_.x]!=Cell::depot || inventory_==0) return false; harvested_+=inventory_; inventory_=0; return true; } return false; }
SimulationSnapshot Simulation::snapshot() const { return {robot_, inventory_, harvested_, harvested_ >= target_}; }
}
