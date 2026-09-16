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
  expect(app.campaign().lessons().size() == 100, "campaign has one hundred lessons");
  expect(app.start_lesson(1), "known lesson starts");
  expect(!app.start_lesson(101), "unknown lesson does not start");
  const auto lesson_15 = app.campaign().find(15);
  expect(lesson_15 && lesson_15->required_construct == "switch", "lesson fifteen has data-driven switch requirement");
  const auto lesson_30 = app.campaign().find(30);
  expect(lesson_30 && lesson_30->required_construct == "for*2", "lesson thirty requires nested loop data");
  const auto lesson_40 = app.campaign().find(40);
  expect(lesson_40 && lesson_40->required_construct == "function:run_automation_cycle", "lesson forty requires data-driven function name");
  const auto lesson_50 = app.campaign().find(50);
  expect(lesson_50 && lesson_50->required_construct == "all:std::vector,std::sort", "lesson fifty requires combined STL data");
  const auto lesson_60 = app.campaign().find(60);
  expect(lesson_60 && lesson_60->required_construct == "all:std::unique_ptr,std::make_unique", "lesson sixty requires safe ownership data");
  const auto lesson_70 = app.campaign().find(70);
  expect(lesson_70 && lesson_70->required_construct == "all:template,std::sort", "lesson seventy requires template algorithm data");
  const auto lesson_80 = app.campaign().find(80);
  expect(lesson_80 && lesson_80->required_construct == "all:std::queue,std::priority_queue", "lesson eighty requires data structure data");
  const auto lesson_90 = app.campaign().find(90);
  expect(lesson_90 && lesson_90->required_construct == "all:std::priority_queue,heuristic", "lesson ninety requires pathfinding data");
  const auto lesson_100 = app.campaign().find(100);
  expect(lesson_100 && lesson_100->required_construct == "all:class,virtual,override", "lesson one hundred requires architecture data");

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
  expect(localizer.text("lesson.m04.l40.title") == "ประเมินฟังก์ชันอัตโนมัติ", "Thai lesson forty exists");
  expect(localizer.text("lesson.m05.l50.title") == "ประเมิน STL สำหรับระบบอัตโนมัติ", "Thai lesson fifty exists");
  expect(localizer.text("lesson.m06.l60.title") == "ประเมินความปลอดภัยของทรัพยากร", "Thai lesson sixty exists");
  expect(localizer.text("lesson.m07.l70.title") == "ประเมินเทมเพลตและอัลกอริทึม", "Thai lesson seventy exists");
  expect(localizer.text("lesson.m08.l80.title") == "ประเมินโครงสร้างข้อมูล", "Thai lesson eighty exists");
  expect(localizer.text("lesson.m09.l90.title") == "ประเมินการหาเส้นทาง", "Thai lesson ninety exists");
  expect(localizer.text("lesson.m10.l100.title") == "โครงการระบบอัตโนมัติฉบับสมบูรณ์", "Thai lesson one hundred exists");
  expect(localizer.text("missing.key") == "missing.key", "missing localization key is visible");
  expect(localizer.load_fallback(BOND_DATA_DIR "/localization/en.psv"), "English fallback localization loads");
  expect(localizer.text("ui.status.ready") == "Worker แยกสำหรับการรันโค้ดพร้อมแล้ว โค้ดของผู้เรียนจะไม่ทำงานในโปรเซสของแอปพลิเคชัน", "Thai remains preferred over fallback");
  for (const auto& lesson : app.campaign().lessons()) {
    expect(localizer.text(lesson.title_key) != lesson.title_key, "every lesson title is localized");
    expect(localizer.text(lesson.requirement_key) != lesson.requirement_key, "every lesson requirement is localized");
    expect(localizer.text(lesson.feedback_key) != lesson.feedback_key, "every lesson feedback is localized");
  }

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
  expect(!evaluator.evaluate_source(conditional, "// if (crop) {}", complete).passed, "evaluator ignores required constructs in comments");
  expect(!evaluator.evaluate_source(conditional, "const char* hint = \"if (crop) {}\";", complete).passed, "evaluator ignores required constructs in strings");
  expect(!evaluator.evaluate_source(conditional, "if (crop) {}", {}).passed, "evaluator requires completed simulation");
  const bond::Lesson nested{30, 3, "m03_l30", "", "", 1, "for*2", "feedback.m03.l30.construct"};
  expect(!evaluator.evaluate_source(nested, "for (;;) {}", complete).passed, "nested evaluator rejects a single loop");
  expect(evaluator.evaluate_source(nested, "for (;;) { for (;;) {} }", complete).passed, "nested evaluator accepts two loops");
  const bond::Lesson named_function{31, 4, "m04_l31", "", "", 1, "function:move_to_crop", "feedback.m04.l31.construct"};
  expect(!evaluator.evaluate_source(named_function, "void move_to_cropper() {}", complete).passed, "function evaluator requires the configured function name");
  expect(evaluator.evaluate_source(named_function, "void move_to_crop(int row) {}", complete).passed, "function evaluator accepts a configured function declaration");
  const bond::Lesson combined_stl{50, 5, "m05_l50", "", "", 1, "all:std::vector,std::sort", "feedback.m05.l50.construct"};
  expect(!evaluator.evaluate_source(combined_stl, "std::vector<int> route;", complete).passed, "combined evaluator requires every STL construct");
  expect(evaluator.evaluate_source(combined_stl, "std::vector<int> route; std::sort(route.begin(), route.end());", complete).passed, "combined evaluator accepts every STL construct");
  const bond::Lesson safe_ownership{60, 6, "m06_l60", "", "", 1, "all:std::unique_ptr,std::make_unique", "feedback.m06.l60.construct"};
  expect(!evaluator.evaluate_source(safe_ownership, "std::unique_ptr<int> route;", complete).passed, "ownership evaluator requires construction and ownership");
  expect(evaluator.evaluate_source(safe_ownership, "auto route = std::make_unique<int>(1); std::unique_ptr<int> backup;", complete).passed, "ownership evaluator accepts safe construction and ownership");
  const bond::Lesson template_algorithm{70, 7, "m07_l70", "", "", 1, "all:template,std::sort", "feedback.m07.l70.construct"};
  expect(!evaluator.evaluate_source(template_algorithm, "template <typename T> T first(T value) { return value; }", complete).passed, "template evaluator requires algorithm construct");
  expect(evaluator.evaluate_source(template_algorithm, "template <typename T> void order(T& value) { std::sort(value.begin(), value.end()); }", complete).passed, "template evaluator accepts template and algorithm constructs");
  const bond::Lesson queued_priority{80, 8, "m08_l80", "", "", 1, "all:std::queue,std::priority_queue", "feedback.m08.l80.construct"};
  expect(!evaluator.evaluate_source(queued_priority, "std::queue<int> route;", complete).passed, "data structure evaluator requires every queue type");
  expect(evaluator.evaluate_source(queued_priority, "std::queue<int> route; std::priority_queue<int> urgent;", complete).passed, "data structure evaluator accepts combined queue types");
  const bond::Lesson pathfinding{90, 9, "m09_l90", "", "", 1, "all:std::priority_queue,heuristic", "feedback.m09.l90.construct"};
  expect(!evaluator.evaluate_source(pathfinding, "std::priority_queue<int> frontier;", complete).passed, "pathfinding evaluator requires a heuristic");
  expect(evaluator.evaluate_source(pathfinding, "std::priority_queue<int> frontier; int heuristic = 0;", complete).passed, "pathfinding evaluator accepts frontier and heuristic");
  const bond::Lesson architecture{100, 10, "m10_l100", "", "", 1, "all:class,virtual,override", "feedback.m10.l100.construct"};
  expect(!evaluator.evaluate_source(architecture, "class RoutePlanner { virtual void run(); };", complete).passed, "architecture evaluator requires implementation override");
  expect(evaluator.evaluate_source(architecture, "class RoutePlanner { virtual void run(); }; class SafePlanner : public RoutePlanner { void run() override {} };", complete).passed, "architecture evaluator accepts an interface and override");

  bond::ExecutionLimits limits;
  expect(!bond::WorkerCodeExecutor::is_request_safe({"", {}}, limits), "execution rejects empty source");
  expect(bond::WorkerCodeExecutor::is_request_safe({"int main() {}", {"lesson-11"}}, limits), "execution accepts bounded source request");
  bond::WorkerCodeExecutor unavailable("missing-worker.exe");
  expect(unavailable.compile_and_run({"int main() {}", {}}).rejected, "execution never falls back to the application process");

  return failures == 0 ? 0 : 1;
}
