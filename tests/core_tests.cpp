#include "bond/application.hpp"
#include "bond/daily_exercise.hpp"
#include "bond/evaluation.hpp"
#include "bond/execution.hpp"
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
  expect(app.campaign().lessons().size() == 30, "campaign has thirty lessons");
  expect(app.start_lesson(1), "known lesson starts");
  expect(!app.start_lesson(99), "unknown lesson does not start");
  const auto lesson_15 = app.campaign().find(15);
  expect(lesson_15 && lesson_15->required_construct == "switch", "lesson fifteen has data-driven switch requirement");
  const auto lesson_30 = app.campaign().find(30);
  expect(lesson_30 && lesson_30->required_construct == "for*2", "lesson thirty requires nested loop data");

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
  sim.reset(5);
  expect(sim.snapshot().remaining_crops == 5 && sim.snapshot().actions == 0, "extended route initializes five crops and no actions");
  expect(sim.step(bond::Command::move_north), "extended route reaches first row");
  expect(sim.step(bond::Command::move_east), "extended route reaches first crop");
  expect(!sim.step(bond::Command::move_south), "extended route blocks the direct lower path");
  expect(sim.snapshot().actions == 2, "only successful simulation actions are counted");

  bond::Localizer localizer;
  expect(localizer.load(BOND_DATA_DIR "/localization/th.psv"), "Thai localization loads");
  expect(localizer.text("lesson.m01.l10.title") == "รอบเก็บเกี่ยวอัตโนมัติ", "Thai lesson ten exists");
  expect(localizer.text("lesson.m02.l20.title") == "ประเมินตรรกะควบคุม", "Thai lesson twenty exists");
  expect(localizer.text("lesson.m03.l30.title") == "ประเมินการทำซ้ำ", "Thai lesson thirty exists");
  expect(localizer.text("missing.key") == "missing.key", "missing localization key is visible");
  expect(localizer.load_fallback(BOND_DATA_DIR "/localization/en.psv"), "English fallback localization loads");
  expect(localizer.text("ui.status.ready") == "Worker แยกสำหรับการรันโค้ดพร้อมแล้ว โค้ดของผู้เรียนจะไม่ทำงานในโปรเซสของแอปพลิเคชัน", "Thai remains preferred over fallback");

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

  bond::LessonEvaluator evaluator;
  const bond::Lesson conditional{11, 2, "m02_l11", "", "", 1, "if", "feedback.m02.l11.construct"};
  const bond::SimulationSnapshot complete{{0, 0}, 0, 1, true};
  expect(!evaluator.evaluate_source(conditional, "int main() {}", complete).passed, "evaluator rejects missing required construct");
  expect(evaluator.evaluate_source(conditional, "if (crop) {}", complete).passed, "evaluator accepts required construct and complete simulation");
  expect(!evaluator.evaluate_source(conditional, "if (crop) {}", {}).passed, "evaluator requires completed simulation");
  const bond::Lesson nested{30, 3, "m03_l30", "", "", 1, "for*2", "feedback.m03.l30.construct"};
  expect(!evaluator.evaluate_source(nested, "for (;;) {}", complete).passed, "nested evaluator rejects a single loop");
  expect(evaluator.evaluate_source(nested, "for (;;) { for (;;) {} }", complete).passed, "nested evaluator accepts two loops");

  bond::ExecutionLimits limits;
  expect(!bond::WorkerCodeExecutor::is_request_safe({"", {}}, limits), "execution rejects empty source");
  expect(bond::WorkerCodeExecutor::is_request_safe({"int main() {}", {"lesson-11"}}, limits), "execution accepts bounded source request");
  bond::WorkerCodeExecutor unavailable("missing-worker.exe");
  expect(unavailable.compile_and_run({"int main() {}", {}}).rejected, "execution never falls back to the application process");

  return failures == 0 ? 0 : 1;
}
