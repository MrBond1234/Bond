#include "bond/application.hpp"
#include "bond/daily_exercise.hpp"
#include "bond/localization.hpp"
#include "bond/persistence.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace {
int failures{};
void expect(bool condition, const char* message) {
  if (!condition) {
    std::cerr << "FAILED: " << message << '\n';
    ++failures;
  }
}
}

int main() {
  bond::Application app;
  expect(app.initialize(BOND_DATA_DIR "/lessons/campaign_v1.psv"), "campaign loads");
  expect(app.campaign().lessons().size() == 10, "campaign has ten lessons");
  expect(app.start_lesson(1), "known lesson starts");
  expect(!app.start_lesson(99), "unknown lesson does not start");

  const auto first = bond::generate_daily_exercise(42);
  const auto second = bond::generate_daily_exercise(42);
  expect(first.target_harvest == second.target_harvest, "daily exercise is deterministic");

  bond::Simulation sim;
  sim.reset(1);
  expect(sim.step(bond::Command::move_north), "robot moves north");
  expect(sim.step(bond::Command::move_east), "robot moves east");
  expect(sim.step(bond::Command::harvest), "robot harvests crop");
  expect(sim.step(bond::Command::move_west), "robot returns west");
  expect(sim.step(bond::Command::move_west), "robot reaches depot");
  expect(sim.step(bond::Command::deposit), "robot deposits inventory");
  expect(sim.snapshot().complete, "lesson completes after deposit");

  bond::Localizer localizer;
  expect(localizer.load(BOND_DATA_DIR "/localization/th.psv"), "Thai localization loads");
  expect(localizer.text("lesson.m01.l10.title") == "รอบเก็บเกี่ยวอัตโนมัติ", "Thai lesson ten exists");
  expect(localizer.text("missing.key") == "missing.key", "missing localization key is visible");

  const auto progress_file = std::filesystem::temp_directory_path() / "bond_core_tests_progress.txt";
  const bond::Progress expected{1, {1, 3, 10}};
  expect(bond::ProgressStore::save(progress_file, expected), "progress saves");
  const auto actual = bond::ProgressStore::load(progress_file);
  expect(actual.schema_version == 1 && actual.completed_lessons == expected.completed_lessons, "progress round-trips");
  {
    std::ofstream malformed(progress_file);
    malformed << "version=not-a-number\ncompleted=1,invalid,3,\n";
  }
  const auto recovered = bond::ProgressStore::load(progress_file);
  expect(recovered.schema_version == 1 && recovered.completed_lessons == std::vector<int>({1, 3}), "malformed progress is tolerated");
  std::filesystem::remove(progress_file);

  return failures == 0 ? 0 : 1;
}
