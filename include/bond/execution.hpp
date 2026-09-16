#pragma once
#include <string>
#include <vector>
namespace bond {
struct ExecutionRequest { std::string source; std::vector<std::string> arguments; };
struct ExecutionResult { int exit_code{-1}; bool timed_out{}; std::string compiler_diagnostics; std::string standard_output; };
// Platform implementation must execute outside the application process.
class CodeExecutor { public: virtual ~CodeExecutor() = default; virtual ExecutionResult compile_and_run(const ExecutionRequest&) = 0; };
}
