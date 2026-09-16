#include "bond/application.hpp"
#include "bond/daily_exercise.hpp"
#include <cassert>
int main() { bond::Application app; assert(app.initialize(BOND_DATA_DIR "/lessons/campaign_v1.psv")); assert(app.campaign().lessons().size() == 10); assert(app.start_lesson(1)); const auto a=bond::generate_daily_exercise(42); const auto b=bond::generate_daily_exercise(42); assert(a.target_harvest==b.target_harvest); bond::Simulation sim; sim.reset(1); assert(sim.step(bond::Command::move_north)); assert(sim.step(bond::Command::move_east)); assert(sim.step(bond::Command::harvest)); assert(sim.step(bond::Command::move_west)); assert(sim.step(bond::Command::move_west)); assert(sim.step(bond::Command::deposit)); assert(sim.snapshot().complete); }
