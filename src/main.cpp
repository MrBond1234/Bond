#include "bond/application.hpp"
#include "bond/automation_protocol.hpp"
#include "bond/evaluation.hpp"
#include "bond/execution.hpp"
#include "bond/localization.hpp"
#include "bond/persistence.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <algorithm>
#include <array>
#include <filesystem>
#include <string>
#include <vector>

namespace {
constexpr int id_run_code = 101;
constexpr int id_reset = 103;
constexpr int id_load_lesson = 104;
constexpr int id_lesson_input = 105;
constexpr int id_move_up = 110;
constexpr int id_move_left = 111;
constexpr int id_harvest = 112;
constexpr int id_move_right = 113;
constexpr int id_move_down = 114;
constexpr int id_deposit = 115;
constexpr int id_check_solution = 116;
constexpr int id_previous_lesson = 117;
constexpr int id_next_lesson = 118;

constexpr COLORREF color_background = RGB(12, 20, 30);
constexpr COLORREF color_panel = RGB(23, 35, 49);
constexpr COLORREF color_editor = RGB(16, 28, 42);
constexpr COLORREF color_text = RGB(229, 237, 245);
constexpr COLORREF color_muted = RGB(159, 177, 196);
constexpr COLORREF color_primary = RGB(37, 99, 235);
constexpr COLORREF color_secondary = RGB(42, 59, 76);

bond::Application application;
bond::Localizer localizer;
bond::LessonEvaluator evaluator;
bond::Progress progress;
std::filesystem::path progress_file;
std::filesystem::path data_directory;
std::filesystem::path execution_worker;

HWND task_pane{};
HWND output_pane{};
HWND simulation_board{};
HWND code_editor{};
HWND lesson_input{};
HWND lesson_label{};
HWND progress_label{};
HWND run_button{};
HFONT title_font{};
HFONT section_font{};
HFONT body_font{};
HFONT code_font{};
HBRUSH background_brush{};
HBRUSH panel_brush{};
HBRUSH editor_brush{};
int active_lesson{1};

std::wstring utf8_to_wide(const std::string& value) {
  if (value.empty()) return {};
  const int size = MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0);
  std::wstring result(size, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), size);
  return result;
}

std::string wide_to_utf8(const std::wstring& value) {
  if (value.empty()) return {};
  const int size = WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
  std::string result(size, '\0');
  WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), size, nullptr, nullptr);
  return result;
}

std::wstring text(std::string_view key) { return utf8_to_wide(localizer.text(key)); }

void set_font(HWND control, HFONT font) {
  SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
}

std::filesystem::path executable_directory() {
  std::array<wchar_t, 32768> path{};
  const auto length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
  return length == 0 ? std::filesystem::current_path() : std::filesystem::path(std::wstring(path.data(), length)).parent_path();
}

std::filesystem::path locate_data_directory() {
  const auto beside_executable = executable_directory() / L"data";
  if (std::filesystem::is_regular_file(beside_executable / "lessons" / "campaign_v1.psv")) return beside_executable;
  return std::filesystem::path(BOND_DATA_DIR);
}

std::filesystem::path locate_worker() {
  const auto beside_executable = executable_directory() / L"bond_execution_worker.exe";
  if (std::filesystem::is_regular_file(beside_executable)) return beside_executable;
  return std::filesystem::path(BOND_EXECUTION_WORKER_PATH);
}

std::filesystem::path locate_progress_file() {
  std::array<wchar_t, 32768> local_app_data{};
  const auto length = GetEnvironmentVariableW(L"LOCALAPPDATA", local_app_data.data(), static_cast<DWORD>(local_app_data.size()));
  std::filesystem::path folder = length == 0 || length >= local_app_data.size()
      ? std::filesystem::temp_directory_path() : std::filesystem::path(std::wstring(local_app_data.data(), length));
  folder /= L"BondAutomationTraining";
  std::error_code error;
  std::filesystem::create_directories(folder, error);
  return folder / L"progress.txt";
}

std::wstring read_control_text(HWND control) {
  const int length = GetWindowTextLengthW(control);
  std::wstring value(static_cast<std::size_t>(length) + 1, L'\0');
  GetWindowTextW(control, value.data(), length + 1);
  value.resize(static_cast<std::size_t>(length));
  return value;
}

void set_output(const std::wstring& value) { SetWindowTextW(output_pane, value.c_str()); }

std::size_t completed_count() {
  std::vector<int> ids = progress.completed_lessons;
  std::sort(ids.begin(), ids.end());
  ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
  return ids.size();
}

bool is_completed(int lesson_id) {
  return std::find(progress.completed_lessons.begin(), progress.completed_lessons.end(), lesson_id) != progress.completed_lessons.end();
}

void refresh_progress() {
  if (!progress_label) return;
  const std::wstring value = text("ui.progress") + L": " + std::to_wstring(completed_count()) + L" / 100";
  SetWindowTextW(progress_label, value.c_str());
}

void refresh_board() {
  if (simulation_board) InvalidateRect(simulation_board, nullptr, TRUE);
}

std::string starter_source() {
  return R"(#include <iostream>

int main() {
    std::cout << "BOND:UP\n";
    std::cout << "BOND:RIGHT\n";
    std::cout << "BOND:HARVEST\n";
    std::cout << "BOND:LEFT\n";
    std::cout << "BOND:LEFT\n";
    std::cout << "BOND:DEPOSIT\n";
    return 0;
}
)";
}

void refresh_task(bool replace_starter_code) {
  const auto lesson = application.campaign().find(active_lesson);
  if (!lesson) {
    SetWindowTextW(task_pane, text("ui.status.invalid_lesson").c_str());
    return;
  }
  const std::wstring completed = is_completed(active_lesson) ? L"  " + text("ui.completed") : L"";
  const std::wstring task = text(lesson->title_key) + completed + L"\r\n\r\n" + text(lesson->requirement_key) +
      L"\r\n\r\n" + text("ui.objective") + L": " + std::to_wstring(lesson->target_harvest) + L" " + text("ui.harvest_units");
  SetWindowTextW(task_pane, task.c_str());
  const std::wstring label = text("ui.lesson") + L" " + std::to_wstring(active_lesson) + L" / 100";
  SetWindowTextW(lesson_label, label.c_str());
  SetWindowTextW(lesson_input, std::to_wstring(active_lesson).c_str());
  if (replace_starter_code) SetWindowTextW(code_editor, utf8_to_wide(starter_source()).c_str());
  refresh_progress();
}

void select_lesson(int lesson_id, bool replace_starter_code = true) {
  if (!application.start_lesson(lesson_id)) {
    set_output(text("ui.status.invalid_lesson"));
    return;
  }
  active_lesson = lesson_id;
  refresh_task(replace_starter_code);
  refresh_board();
  set_output(text("ui.status.lesson_loaded"));
}

void apply_command(bond::Command command, bool show_feedback = true) {
  const bool accepted = application.step_simulation(command);
  refresh_board();
  if (!show_feedback) return;
  const auto snapshot = application.simulation().snapshot();
  if (!accepted) {
    set_output(text("ui.status.action_blocked"));
  } else if (snapshot.complete) {
    set_output(text("ui.status.simulation_complete"));
  } else {
    set_output(text("ui.status.action_applied"));
  }
}

void run_code() {
  if (!std::filesystem::is_regular_file(execution_worker)) {
    set_output(text("ui.status.worker_missing"));
    return;
  }
  bond::WorkerCodeExecutor executor(execution_worker);
  const auto result = executor.compile_and_run({wide_to_utf8(read_control_text(code_editor)), {"lesson-" + std::to_string(active_lesson)}});
  if (result.rejected || !result.compiler_diagnostics.empty()) {
    const std::wstring details = utf8_to_wide(result.compiler_diagnostics);
    set_output(text("ui.status.run_failed") + L"\r\n\r\n" + (details.empty() ? text("ui.status.compiler_unavailable") : details));
    return;
  }
  const auto parsed = bond::parse_automation_commands(result.standard_output);
  if (parsed.commands.empty()) {
    set_output(text("ui.status.no_commands") + L"\r\n\r\n" + utf8_to_wide(result.standard_output));
    return;
  }
  application.start_lesson(active_lesson);
  for (const auto command : parsed.commands) application.step_simulation(command);
  refresh_board();
  std::wstring report = text("ui.status.commands_applied") + L": " + std::to_wstring(parsed.commands.size());
  if (parsed.ignored_lines != 0) report += L"\r\n" + text("ui.status.commands_ignored") + L": " + std::to_wstring(parsed.ignored_lines);
  if (parsed.limit_reached) report += L"\r\n" + text("ui.status.command_limit");
  if (application.simulation().snapshot().complete) report += L"\r\n\r\n" + text("ui.status.simulation_complete");
  set_output(report);
}

void check_solution() {
  const auto lesson = application.campaign().find(active_lesson);
  if (!lesson) return;
  const auto evaluation = evaluator.evaluate_source(*lesson, wide_to_utf8(read_control_text(code_editor)), application.simulation().snapshot());
  if (!evaluation.passed) {
    std::wstring report = text("ui.status.not_complete");
    for (const auto& key : evaluation.feedback_keys) report += L"\r\n\r\n" + text(key);
    set_output(report);
    return;
  }
  if (!is_completed(active_lesson)) progress.completed_lessons.push_back(active_lesson);
  std::sort(progress.completed_lessons.begin(), progress.completed_lessons.end());
  progress.completed_lessons.erase(std::unique(progress.completed_lessons.begin(), progress.completed_lessons.end()), progress.completed_lessons.end());
  const bool saved = bond::ProgressStore::save(progress_file, progress);
  refresh_task(false);
  set_output(saved ? text("ui.status.lesson_complete") : text("ui.status.lesson_complete_unsaved"));
}

COLORREF cell_color(bond::Cell cell) {
  switch (cell) {
    case bond::Cell::depot: return RGB(30, 95, 165);
    case bond::Cell::crop: return RGB(22, 125, 83);
    case bond::Cell::wall: return RGB(75, 85, 99);
    case bond::Cell::empty: return RGB(31, 47, 63);
  }
  return color_panel;
}

LRESULT CALLBACK board_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
  if (message == WM_PAINT) {
    PAINTSTRUCT paint{};
    HDC device = BeginPaint(window, &paint);
    RECT area{};
    GetClientRect(window, &area);
    FillRect(device, &area, panel_brush);
    const auto& simulation = application.simulation();
    const int available_width = static_cast<int>(area.right - area.left);
    const int available_height = static_cast<int>(area.bottom - area.top);
    const int cell_size = std::max(24, std::min((available_width - 20) / simulation.width(),
                                                 (available_height - 20) / simulation.height()));
    const int grid_width = cell_size * simulation.width();
    const int grid_height = cell_size * simulation.height();
    const int origin_x = std::max(10, (available_width - grid_width) / 2);
    const int origin_y = std::max(10, (available_height - grid_height) / 2);
    SetBkMode(device, TRANSPARENT);
    for (int y = 0; y < simulation.height(); ++y) {
      for (int x = 0; x < simulation.width(); ++x) {
        RECT cell{origin_x + x * cell_size, origin_y + y * cell_size, origin_x + (x + 1) * cell_size, origin_y + (y + 1) * cell_size};
        const auto type = simulation.cell_at({x, y});
        HBRUSH brush = CreateSolidBrush(cell_color(type));
        FillRect(device, &cell, brush);
        DeleteObject(brush);
        FrameRect(device, &cell, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
        const wchar_t* symbol = type == bond::Cell::depot ? L"D" : type == bond::Cell::crop ? L"C" : L"";
        SetTextColor(device, color_text);
        DrawTextW(device, symbol, -1, &cell, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
      }
    }
    const auto robot = simulation.snapshot().robot;
    RECT robot_rect{origin_x + robot.x * cell_size + cell_size / 5, origin_y + robot.y * cell_size + cell_size / 5,
                    origin_x + (robot.x + 1) * cell_size - cell_size / 5, origin_y + (robot.y + 1) * cell_size - cell_size / 5};
    HBRUSH robot_brush = CreateSolidBrush(RGB(245, 158, 11));
    FillRect(device, &robot_rect, robot_brush);
    DeleteObject(robot_brush);
    SetTextColor(device, RGB(15, 23, 42));
    DrawTextW(device, L"R", -1, &robot_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    EndPaint(window, &paint);
    return 0;
  }
  return DefWindowProcW(window, message, wparam, lparam);
}

HWND create_label(HWND parent, std::string_view label, HFONT font) {
  HWND control = CreateWindowW(L"STATIC", text(label).c_str(), WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, parent, nullptr, nullptr, nullptr);
  set_font(control, font);
  return control;
}

HWND create_button(HWND parent, std::string_view label, int id) {
  HWND control = CreateWindowW(L"BUTTON", text(label).c_str(), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
                               0, 0, 0, 0, parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), nullptr, nullptr);
  set_font(control, body_font);
  return control;
}

void move_control(HWND control, int x, int y, int width, int height) {
  MoveWindow(control, x, y, width, height, TRUE);
}

void layout_controls(HWND window) {
  if (!code_editor) return;
  RECT client{};
  GetClientRect(window, &client);
  const int width = client.right;
  const int height = client.bottom;
  const int margin = 22;
  const int gap = 12;
  const int left_width = 410;
  const int right_x = margin + left_width + gap;
  const int right_width = std::max(480, width - right_x - margin);
  const int header_y = 82;
  const int task_height = 130;
  const int board_y = header_y + task_height + 58;
  const int board_size = std::min(350, std::max(260, height - board_y - 150));
  const int code_y = 112;
  const int output_height = std::max(120, height - 620);
  const int output_y = height - output_height - margin;
  const int code_height = std::max(260, output_y - code_y - 92);

  move_control(lesson_label, margin, 20, 250, 28);
  move_control(lesson_input, margin + 260, 20, 54, 28);
  move_control(GetDlgItem(window, id_load_lesson), margin + 322, 20, 88, 28);
  move_control(GetDlgItem(window, id_previous_lesson), width - 306, 20, 132, 28);
  move_control(GetDlgItem(window, id_next_lesson), width - 164, 20, 142, 28);
  move_control(progress_label, right_x, 54, right_width, 22);
  move_control(GetDlgItem(window, 200), margin, header_y, left_width, 22);
  move_control(task_pane, margin, header_y + 26, left_width, task_height);
  move_control(GetDlgItem(window, 201), margin, board_y - 26, left_width, 22);
  move_control(simulation_board, margin, board_y, left_width, board_size);

  const int controls_y = board_y + board_size + 10;
  const int square = 52;
  move_control(GetDlgItem(window, id_move_up), margin + 160, controls_y, square, 32);
  move_control(GetDlgItem(window, id_move_left), margin + 104, controls_y + 38, square, 32);
  move_control(GetDlgItem(window, id_harvest), margin + 160, controls_y + 38, square, 32);
  move_control(GetDlgItem(window, id_move_right), margin + 216, controls_y + 38, square, 32);
  move_control(GetDlgItem(window, id_move_down), margin + 160, controls_y + 76, square, 32);
  move_control(GetDlgItem(window, id_deposit), margin + 228, controls_y + 76, 142, 32);
  move_control(GetDlgItem(window, id_reset), margin, controls_y + 76, 142, 32);

  move_control(GetDlgItem(window, 202), right_x, header_y, right_width, 22);
  move_control(code_editor, right_x, code_y, right_width, code_height);
  const int action_y = code_y + code_height + 12;
  move_control(run_button, right_x, action_y, 148, 34);
  move_control(GetDlgItem(window, id_check_solution), right_x + 160, action_y, 170, 34);
  move_control(GetDlgItem(window, 203), right_x, output_y - 26, right_width, 22);
  move_control(output_pane, right_x, output_y, right_width, output_height);
}

void draw_button(const DRAWITEMSTRUCT& item) {
  const bool primary = item.CtlID == id_run_code || item.CtlID == id_check_solution;
  const bool active = (item.itemState & ODS_SELECTED) != 0;
  const bool disabled = (item.itemState & ODS_DISABLED) != 0;
  const COLORREF fill = disabled ? RGB(55, 65, 81) : primary ? (active ? RGB(29, 78, 216) : color_primary) : (active ? RGB(55, 75, 94) : color_secondary);
  HBRUSH brush = CreateSolidBrush(fill);
  FillRect(item.hDC, &item.rcItem, brush);
  DeleteObject(brush);
  SetBkMode(item.hDC, TRANSPARENT);
  SetTextColor(item.hDC, disabled ? color_muted : color_text);
  wchar_t label[128]{};
  GetWindowTextW(item.hwndItem, label, static_cast<int>(std::size(label)));
  DrawTextW(item.hDC, label, -1, const_cast<RECT*>(&item.rcItem), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  if ((item.itemState & ODS_FOCUS) != 0) DrawFocusRect(item.hDC, &item.rcItem);
}

void show_startup_error(const std::wstring& detail) {
  const std::wstring heading = text("ui.startup_error");
  const std::wstring body = text("ui.status.startup_failed") + L"\r\n\r\n" + detail;
  MessageBoxW(nullptr, body.c_str(), heading.empty() ? L"Bond Automation Training" : heading.c_str(), MB_OK | MB_ICONERROR);
}

LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
  switch (message) {
    case WM_COMMAND: {
      switch (LOWORD(wparam)) {
        case id_run_code: run_code(); return 0;
        case id_reset: select_lesson(active_lesson, false); return 0;
        case id_load_lesson: {
          BOOL converted{};
          const int selected = static_cast<int>(GetDlgItemInt(window, id_lesson_input, &converted, FALSE));
          if (converted) select_lesson(selected);
          else set_output(text("ui.status.invalid_lesson"));
          return 0;
        }
        case id_previous_lesson: if (active_lesson > 1) select_lesson(active_lesson - 1); return 0;
        case id_next_lesson: if (active_lesson < 100) select_lesson(active_lesson + 1); return 0;
        case id_move_up: apply_command(bond::Command::move_north); return 0;
        case id_move_down: apply_command(bond::Command::move_south); return 0;
        case id_move_left: apply_command(bond::Command::move_west); return 0;
        case id_move_right: apply_command(bond::Command::move_east); return 0;
        case id_harvest: apply_command(bond::Command::harvest); return 0;
        case id_deposit: apply_command(bond::Command::deposit); return 0;
        case id_check_solution: check_solution(); return 0;
      }
      break;
    }
    case WM_KEYDOWN:
      if (wparam == VK_UP) { apply_command(bond::Command::move_north); return 0; }
      if (wparam == VK_DOWN) { apply_command(bond::Command::move_south); return 0; }
      if (wparam == VK_LEFT) { apply_command(bond::Command::move_west); return 0; }
      if (wparam == VK_RIGHT) { apply_command(bond::Command::move_east); return 0; }
      if (wparam == 'H') { apply_command(bond::Command::harvest); return 0; }
      if (wparam == 'D') { apply_command(bond::Command::deposit); return 0; }
      break;
    case WM_DRAWITEM:
      if (wparam != 0) { draw_button(*reinterpret_cast<DRAWITEMSTRUCT*>(lparam)); return TRUE; }
      break;
    case WM_CTLCOLOREDIT: {
      HDC device = reinterpret_cast<HDC>(wparam);
      SetTextColor(device, color_text);
      SetBkColor(device, color_editor);
      return reinterpret_cast<LRESULT>(editor_brush);
    }
    case WM_CTLCOLORSTATIC: {
      HDC device = reinterpret_cast<HDC>(wparam);
      SetTextColor(device, color_text);
      SetBkColor(device, color_background);
      return reinterpret_cast<LRESULT>(background_brush);
    }
    case WM_SIZE: layout_controls(window); return 0;
    case WM_GETMINMAXINFO: {
      auto* info = reinterpret_cast<MINMAXINFO*>(lparam);
      info->ptMinTrackSize = {1120, 760};
      return 0;
    }
    case WM_DESTROY:
      DeleteObject(title_font);
      DeleteObject(section_font);
      DeleteObject(body_font);
      DeleteObject(code_font);
      DeleteObject(background_brush);
      DeleteObject(panel_brush);
      DeleteObject(editor_brush);
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProcW(window, message, wparam, lparam);
}
}

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int command_show) {
  SetProcessDPIAware();
  data_directory = locate_data_directory();
  const bool campaign_loaded = application.initialize(data_directory / "lessons" / "campaign_v1.psv");
  const bool fallback_loaded = localizer.load_fallback(data_directory / "localization" / "en.psv");
  localizer.load(data_directory / "localization" / "th.psv");
  if (!campaign_loaded || !fallback_loaded) {
    show_startup_error(data_directory.wstring());
    return 1;
  }
  execution_worker = locate_worker();
  progress_file = locate_progress_file();
  progress = bond::ProgressStore::load(progress_file);
  if (!progress.completed_lessons.empty()) {
    const int latest = *std::max_element(progress.completed_lessons.begin(), progress.completed_lessons.end());
    active_lesson = std::min(100, latest + 1);
  }
  application.start_lesson(active_lesson);

  background_brush = CreateSolidBrush(color_background);
  panel_brush = CreateSolidBrush(color_panel);
  editor_brush = CreateSolidBrush(color_editor);
  title_font = CreateFontW(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                           CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
  section_font = CreateFontW(15, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
  body_font = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                          CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
  code_font = CreateFontW(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                          CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH, L"Cascadia Mono");

  const wchar_t window_class_name[] = L"BondTrainingWorkbench";
  WNDCLASSW window_class{};
  window_class.hInstance = instance;
  window_class.lpszClassName = window_class_name;
  window_class.lpfnWndProc = window_proc;
  window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
  window_class.hbrBackground = background_brush;
  if (RegisterClassW(&window_class) == 0) {
    show_startup_error(text("ui.status.window_failed"));
    return 2;
  }
  WNDCLASSW board_class{};
  board_class.hInstance = instance;
  board_class.lpszClassName = L"BondSimulationBoard";
  board_class.lpfnWndProc = board_proc;
  board_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
  board_class.hbrBackground = panel_brush;
  RegisterClassW(&board_class);

  HWND window = CreateWindowExW(0, window_class_name, text("app.title").c_str(), WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                 CW_USEDEFAULT, CW_USEDEFAULT, 1280, 840, nullptr, nullptr, instance, nullptr);
  if (!window) {
    show_startup_error(text("ui.status.window_failed"));
    return 3;
  }

  HWND title = CreateWindowW(L"STATIC", text("app.title").c_str(), WS_CHILD | WS_VISIBLE, 22, 52, 410, 28, window, nullptr, instance, nullptr);
  set_font(title, title_font);
  lesson_label = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, window, nullptr, instance, nullptr);
  set_font(lesson_label, section_font);
  lesson_input = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"1", WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_CENTER, 0, 0, 0, 0,
                                  window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id_lesson_input)), instance, nullptr);
  set_font(lesson_input, body_font);
  create_button(window, "ui.load_lesson", id_load_lesson);
  create_button(window, "ui.previous", id_previous_lesson);
  create_button(window, "ui.next", id_next_lesson);
  progress_label = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_RIGHT, 0, 0, 0, 0, window, nullptr, instance, nullptr);
  set_font(progress_label, body_font);
  HWND task_heading = create_label(window, "ui.task", section_font); SetWindowLongPtrW(task_heading, GWLP_ID, 200);
  task_pane = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
                               0, 0, 0, 0, window, nullptr, instance, nullptr);
  set_font(task_pane, body_font);
  HWND board_heading = create_label(window, "ui.simulation", section_font); SetWindowLongPtrW(board_heading, GWLP_ID, 201);
  simulation_board = CreateWindowW(L"BondSimulationBoard", L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, window, nullptr, instance, nullptr);
  create_button(window, "ui.up", id_move_up);
  create_button(window, "ui.left", id_move_left);
  create_button(window, "ui.harvest", id_harvest);
  create_button(window, "ui.right", id_move_right);
  create_button(window, "ui.down", id_move_down);
  create_button(window, "ui.deposit", id_deposit);
  create_button(window, "ui.reset", id_reset);
  HWND code_heading = create_label(window, "ui.code_editor", section_font); SetWindowLongPtrW(code_heading, GWLP_ID, 202);
  code_editor = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
                                 0, 0, 0, 0, window, nullptr, instance, nullptr);
  set_font(code_editor, code_font);
  run_button = create_button(window, "ui.run_code", id_run_code);
  create_button(window, "ui.check_solution", id_check_solution);
  HWND output_heading = create_label(window, "ui.output", section_font); SetWindowLongPtrW(output_heading, GWLP_ID, 203);
  output_pane = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
                                 0, 0, 0, 0, window, nullptr, instance, nullptr);
  set_font(output_pane, code_font);

  refresh_task(true);
  refresh_board();
  set_output(std::filesystem::is_regular_file(execution_worker) ? text("ui.status.ready") : text("ui.status.worker_missing"));
  layout_controls(window);
  ShowWindow(window, command_show == SW_HIDE ? SW_SHOWNORMAL : command_show);
  UpdateWindow(window);
  MSG message{};
  while (true) {
    const int result = GetMessageW(&message, nullptr, 0, 0);
    if (result <= 0) return result == 0 ? 0 : 4;
    TranslateMessage(&message);
    DispatchMessageW(&message);
  }
}
