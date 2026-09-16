#include "bond/application.hpp"
#include "bond/localization.hpp"
#include "bond/execution.hpp"
#include <windows.h>
#include <string>

namespace {
bond::Application application;
bond::Localizer localizer;
HWND output_pane{};
HWND simulation_pane{};
HWND code_editor{};
HWND task_pane{};
int active_lesson{91};
std::wstring utf8_to_wide(const std::string& value) {
  if (value.empty()) return {};
  const int size = MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0);
  std::wstring result(size, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), size);
  return result;
}
std::wstring text(const char* key) { return utf8_to_wide(localizer.text(key)); }
std::string wide_to_utf8(const std::wstring& value) {
  const int size = WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
  std::string result(size, '\0');
  WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), size, nullptr, nullptr);
  return result;
}
HWND panel(HWND parent, const wchar_t* heading, int x, int y, int width, int height) {
  CreateWindowW(L"STATIC", heading, WS_CHILD | WS_VISIBLE, x, y, width, 24, parent, nullptr, nullptr, nullptr);
  return CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
                         x, y + 26, width, height - 26, parent, nullptr, nullptr, nullptr);
}
void refresh_snapshot() {
  const auto state = application.simulation().snapshot();
  const std::wstring value = L"Robot position: (" + std::to_wstring(state.robot.x) + L", " + std::to_wstring(state.robot.y) +
      L")\r\nInventory: " + std::to_wstring(state.inventory) + L"\r\nDelivered: " + std::to_wstring(state.harvested);
  SetWindowTextW(simulation_pane, value.c_str());
}
void refresh_task() {
  const auto lesson = application.campaign().find(active_lesson);
  const std::wstring task = lesson ? text(lesson->title_key.c_str()) + L"\r\n\r\n" + text(lesson->requirement_key.c_str()) : L"Campaign data unavailable.";
  SetWindowTextW(task_pane, task.c_str());
}
LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM) {
  if (message == WM_COMMAND) {
    switch (LOWORD(wparam)) {
      case 101: {
        const int length = GetWindowTextLengthW(code_editor); std::wstring source(length + 1, L'\0');
        GetWindowTextW(code_editor, source.data(), length + 1); source.resize(length);
        bond::WorkerCodeExecutor executor(BOND_EXECUTION_WORKER_PATH);
        const auto result = executor.compile_and_run({wide_to_utf8(source), {"lesson-" + std::to_string(active_lesson)}});
        const auto report = result.compiler_diagnostics.empty() ? result.standard_output : result.compiler_diagnostics + "\r\n" + result.standard_output;
        SetWindowTextW(output_pane, utf8_to_wide(report.empty() ? localizer.text("ui.status.no_output") : report).c_str()); break;
      }
      case 102: application.step_simulation(bond::Command::move_north); refresh_snapshot(); SetWindowTextW(output_pane, text("ui.status.stepped").c_str()); break;
      case 103: application.start_lesson(active_lesson); refresh_snapshot(); SetWindowTextW(output_pane, text("ui.status.reset").c_str()); break;
      case 104: {
        const int selected = static_cast<int>(GetDlgItemInt(window, 105, nullptr, FALSE));
        if (application.start_lesson(selected)) { active_lesson = selected; refresh_task(); refresh_snapshot(); SetWindowTextW(output_pane, text("ui.status.reset").c_str()); }
        else SetWindowTextW(output_pane, text("ui.status.invalid_lesson").c_str());
        break;
      }
    }
    return 0;
  }
  if (message == WM_DESTROY) { PostQuitMessage(0); return 0; }
  return DefWindowProcW(window, message, wparam, 0);
}
}

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int command_show) {
  application.initialize(BOND_DATA_DIR "/lessons/campaign_v1.psv");
  localizer.load_fallback(BOND_DATA_DIR "/localization/en.psv");
  localizer.load(BOND_DATA_DIR "/localization/th.psv");
  application.start_lesson(active_lesson);
  const wchar_t class_name[] = L"BondTrainingWorkbench";
  WNDCLASSW window_class{}; window_class.hInstance = instance; window_class.lpszClassName = class_name;
  window_class.lpfnWndProc = window_proc; window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
  window_class.hbrBackground = static_cast<HBRUSH>(GetStockObject(DKGRAY_BRUSH)); RegisterClassW(&window_class);
  HWND window = CreateWindowExW(0, class_name, text("app.title").c_str(), WS_OVERLAPPEDWINDOW | WS_VISIBLE,
      CW_USEDEFAULT, CW_USEDEFAULT, 1180, 720, nullptr, nullptr, instance, nullptr);
  CreateWindowW(L"STATIC", text("ui.lesson").c_str(), WS_CHILD | WS_VISIBLE, 20, 20, 110, 24, window, nullptr, instance, nullptr);
  CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"91", WS_CHILD | WS_VISIBLE | ES_NUMBER, 130, 20, 70, 24, window, reinterpret_cast<HMENU>(105), instance, nullptr);
  CreateWindowW(L"BUTTON", text("ui.load_lesson").c_str(), WS_CHILD | WS_VISIBLE, 210, 20, 140, 24, window, reinterpret_cast<HMENU>(104), instance, nullptr);
  CreateWindowW(L"STATIC", text("ui.task").c_str(), WS_CHILD | WS_VISIBLE, 20, 54, 330, 24, window, nullptr, instance, nullptr);
  task_pane = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY, 20, 80, 330, 160, window, nullptr, instance, nullptr);
  refresh_task();
  simulation_pane = panel(window, text("ui.simulation").c_str(), 20, 270, 330, 310); refresh_snapshot();
  CreateWindowW(L"BUTTON", text("ui.run").c_str(), WS_CHILD | WS_VISIBLE, 20, 610, 100, 32, window, reinterpret_cast<HMENU>(101), instance, nullptr);
  CreateWindowW(L"BUTTON", text("ui.step").c_str(), WS_CHILD | WS_VISIBLE, 130, 610, 100, 32, window, reinterpret_cast<HMENU>(102), instance, nullptr);
  CreateWindowW(L"BUTTON", text("ui.reset").c_str(), WS_CHILD | WS_VISIBLE, 240, 610, 100, 32, window, reinterpret_cast<HMENU>(103), instance, nullptr);
  CreateWindowW(L"STATIC", text("ui.code_editor").c_str(), WS_CHILD | WS_VISIBLE, 380, 20, 760, 24, window, nullptr, instance, nullptr);
  code_editor = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"// Write C++ automation here\r\nint main() { return 0; }", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, 380, 46, 760, 320, window, nullptr, instance, nullptr);
  output_pane = panel(window, text("ui.output").c_str(), 380, 400, 760, 240);
  SetWindowTextW(output_pane, text("ui.status.ready").c_str());
  ShowWindow(window, command_show); UpdateWindow(window);
  MSG message{}; while (GetMessageW(&message, nullptr, 0, 0)) { TranslateMessage(&message); DispatchMessageW(&message); }
  return 0;
}
