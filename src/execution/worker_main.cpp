#include <windows.h>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

namespace {
void write_result(const std::filesystem::path& path, const std::string& diagnostics, const std::string& output) {
  std::ofstream out(path, std::ios::binary); out << diagnostics << '\0' << output << '\0';
}
std::string read_pipe_output(HANDLE pipe, std::size_t maximum) {
  std::string output; char buffer[512]; DWORD count{};
  while (ReadFile(pipe, buffer, sizeof(buffer), &count, nullptr) && count > 0) {
    output.append(buffer, count);
    if (output.size() >= maximum) { output.resize(maximum); break; }
  }
  return output;
}
bool run_process(std::wstring command, const std::filesystem::path& directory, DWORD timeout_ms,
                 std::size_t output_limit, std::size_t memory_limit, std::string& output, bool restricted) {
  SECURITY_ATTRIBUTES security{sizeof(security), nullptr, TRUE}; HANDLE read_pipe{}, write_pipe{};
  if (!CreatePipe(&read_pipe, &write_pipe, &security, 0)) return false;
  SetHandleInformation(read_pipe, HANDLE_FLAG_INHERIT, 0);
  STARTUPINFOW startup{}; startup.cb = sizeof(startup); startup.dwFlags = STARTF_USESTDHANDLES;
  startup.hStdOutput = write_pipe; startup.hStdError = write_pipe; startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
  PROCESS_INFORMATION process{};
  const DWORD flags = CREATE_NO_WINDOW | (restricted ? CREATE_SUSPENDED : 0);
  if (!CreateProcessW(nullptr, command.data(), nullptr, nullptr, TRUE, flags, nullptr, directory.c_str(), &startup, &process)) {
    CloseHandle(read_pipe); CloseHandle(write_pipe); return false;
  }
  HANDLE job{};
  if (restricted) {
    job = CreateJobObjectW(nullptr, nullptr); JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
    limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE | JOB_OBJECT_LIMIT_PROCESS_MEMORY | JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
    limits.ProcessMemoryLimit = memory_limit; limits.BasicLimitInformation.ActiveProcessLimit = 1;
    SetInformationJobObject(job, JobObjectExtendedLimitInformation, &limits, sizeof(limits));
    AssignProcessToJobObject(job, process.hProcess); ResumeThread(process.hThread);
  }
  CloseHandle(write_pipe);
  const DWORD waited = WaitForSingleObject(process.hProcess, timeout_ms);
  if (waited == WAIT_TIMEOUT) TerminateProcess(process.hProcess, 1);
  output = read_pipe_output(read_pipe, output_limit);
  CloseHandle(read_pipe); CloseHandle(process.hThread); CloseHandle(process.hProcess); if (job) CloseHandle(job);
  return waited != WAIT_TIMEOUT;
}

std::optional<std::filesystem::path> visual_studio_environment_script() {
  wchar_t program_files[32768]{};
  const auto length = GetEnvironmentVariableW(L"ProgramFiles(x86)", program_files, static_cast<DWORD>(std::size(program_files)));
  if (length == 0 || length >= std::size(program_files)) return std::nullopt;
  const std::filesystem::path root = std::filesystem::path(std::wstring(program_files, length)) / L"Microsoft Visual Studio";
  constexpr const wchar_t* editions[]{L"BuildTools", L"Community", L"Professional", L"Enterprise"};
  constexpr const wchar_t* versions[]{L"18", L"2022", L"2019"};
  for (const auto* version : versions) {
    for (const auto* edition : editions) {
      const auto script = root / version / edition / L"Common7" / L"Tools" / L"VsDevCmd.bat";
      if (std::filesystem::is_regular_file(script)) return script;
    }
  }
  return std::nullopt;
}

bool compile_with_visual_studio(const std::wstring& compiler_command, const std::filesystem::path& directory,
                                DWORD timeout_ms, std::size_t output_limit, std::size_t memory_limit,
                                std::string& diagnostics) {
  if (run_process(compiler_command, directory, timeout_ms, output_limit, memory_limit, diagnostics, false)) return true;
  const auto environment_script = visual_studio_environment_script();
  if (!environment_script) return false;
  // The shell receives only a fixed Visual Studio setup script and generated worker paths. Learner source is never interpolated.
  const std::wstring bootstrap = L"cmd.exe /d /s /c \"call \"" + environment_script->wstring() +
      L"\" -arch=x64 >nul && " + compiler_command + L"\"";
  diagnostics.clear();
  return run_process(bootstrap, directory, timeout_ms, output_limit, memory_limit, diagnostics, false);
}
}

int wmain(int argc, wchar_t** argv) {
  if (argc != 3) return 2;
  const std::filesystem::path request(argv[1]), result(argv[2]);
  std::ifstream input(request, std::ios::binary); unsigned int timeout{}; std::size_t memory{}, size{};
  input >> timeout >> memory >> size; input.get();
  std::string source(size, '\0'); input.read(source.data(), static_cast<std::streamsize>(size));
  if (!input || size == 0 || size > 64 * 1024 || timeout == 0 || memory < 8 * 1024 * 1024) {
    write_result(result, "Worker rejected an invalid execution request.", ""); return 3;
  }
  const auto directory = std::filesystem::temp_directory_path() / ("bond-worker-" + std::to_string(GetCurrentProcessId()));
  std::error_code ignored; std::filesystem::create_directories(directory, ignored);
  const auto source_file = directory / "learner.cpp"; const auto program = directory / "learner.exe";
  { std::ofstream out(source_file, std::ios::binary); out << source; }
  std::string diagnostics, output;
  const std::wstring compiler = L"cl.exe /nologo /EHsc /std:c++20 /MT \"" + source_file.wstring() + L"\" /Fe:\"" + program.wstring() + L"\"";
  if (!compile_with_visual_studio(compiler, directory, timeout, 16 * 1024, memory, diagnostics) || !std::filesystem::is_regular_file(program)) {
    write_result(result, diagnostics.empty() ? "C++ compiler was unavailable or compilation timed out." : diagnostics, "");
    std::filesystem::remove_all(directory, ignored); return 4;
  }
  const bool completed = run_process(L"\"" + program.wstring() + L"\"", directory, timeout, 16 * 1024, memory, output, true);
  write_result(result, completed ? "" : "Learner program exceeded its execution time limit.", output);
  std::filesystem::remove_all(directory, ignored);
  return completed ? 0 : 5;
}
