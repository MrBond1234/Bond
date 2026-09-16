#include "bond/execution.hpp"
#include <chrono>
#include <fstream>
#include <random>

#ifdef _WIN32
#include <windows.h>
#endif

namespace bond {
WorkerCodeExecutor::WorkerCodeExecutor(std::filesystem::path worker_path, ExecutionLimits limits)
    : worker_path_(std::move(worker_path)), limits_(limits) {}

bool WorkerCodeExecutor::is_request_safe(const ExecutionRequest& request, const ExecutionLimits& limits) {
  if (request.source.empty() || request.source.size() > limits.max_source_bytes || request.arguments.size() > 16) return false;
  for (const auto& argument : request.arguments) if (argument.size() > 256 || argument.find('\0') != std::string::npos) return false;
  return true;
}

ExecutionResult WorkerCodeExecutor::compile_and_run(const ExecutionRequest& request) {
  ExecutionResult result;
  if (!is_request_safe(request, limits_) || !std::filesystem::is_regular_file(worker_path_)) {
    result.rejected = true;
    result.compiler_diagnostics = "Execution request rejected by the local policy.";
    return result;
  }
  const auto unique = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
  const auto request_file = std::filesystem::temp_directory_path() / ("bond-request-" + unique + ".txt");
  const auto result_file = std::filesystem::temp_directory_path() / ("bond-result-" + unique + ".txt");
  {
    std::ofstream out(request_file, std::ios::binary);
    out << limits_.timeout_ms << '\n' << limits_.max_memory_bytes << '\n' << request.source.size() << '\n' << request.source;
    if (!out) { result.rejected = true; result.compiler_diagnostics = "Unable to create execution request."; return result; }
  }
#ifdef _WIN32
  std::wstring command = L"\"" + worker_path_.wstring() + L"\" \"" + request_file.wstring() + L"\" \"" + result_file.wstring() + L"\"";
  STARTUPINFOW startup{}; startup.cb = sizeof(startup);
  PROCESS_INFORMATION process{};
  if (!CreateProcessW(worker_path_.c_str(), command.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process)) {
    result.rejected = true; result.compiler_diagnostics = "Unable to start the isolated execution worker.";
  } else {
    const auto wait = WaitForSingleObject(process.hProcess, limits_.timeout_ms + 1000U);
    if (wait == WAIT_TIMEOUT) { TerminateProcess(process.hProcess, 1); result.timed_out = true; }
    CloseHandle(process.hThread); CloseHandle(process.hProcess);
    std::ifstream in(result_file, std::ios::binary);
    if (in) { std::getline(in, result.compiler_diagnostics, '\0'); std::getline(in, result.standard_output, '\0'); }
  }
#else
  result.rejected = true; result.compiler_diagnostics = "The isolated Windows execution worker is unavailable on this platform.";
#endif
  std::error_code ignored;
  std::filesystem::remove(request_file, ignored); std::filesystem::remove(result_file, ignored);
  return result;
}
}
