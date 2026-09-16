#pragma once
#include <string>
#include <vector>
#include <filesystem>
namespace bond {
struct ExecutionRequest { std::string source; std::vector<std::string> arguments; };
struct ExecutionLimits { unsigned int timeout_ms{2000}; std::size_t max_source_bytes{64 * 1024}; std::size_t max_output_bytes{16 * 1024}; std::size_t max_memory_bytes{64 * 1024 * 1024}; };
struct ExecutionResult { int exit_code{-1}; bool timed_out{}; bool rejected{}; std::string compiler_diagnostics; std::string standard_output; };
// Platform implementation must execute outside the application process.
class CodeExecutor { public: virtual ~CodeExecutor() = default; virtual ExecutionResult compile_and_run(const ExecutionRequest&) = 0; };
// Writes a request file and launches the dedicated worker directly (never through a shell).
class WorkerCodeExecutor final : public CodeExecutor {
public:
  explicit WorkerCodeExecutor(std::filesystem::path worker_path, ExecutionLimits limits = {});
  [[nodiscard]] ExecutionResult compile_and_run(const ExecutionRequest&) override;
  [[nodiscard]] static bool is_request_safe(const ExecutionRequest&, const ExecutionLimits&);
private:
  std::filesystem::path worker_path_;
  ExecutionLimits limits_;
};
}
