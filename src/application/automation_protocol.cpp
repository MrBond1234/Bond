#include "bond/automation_protocol.hpp"

namespace bond {
namespace {
bool append_command(std::string_view line, std::vector<Command>& commands) {
  const auto append = [&](Command command) { commands.push_back(command); return true; };
  if (line == "BOND:UP") return append(Command::move_north);
  if (line == "BOND:DOWN") return append(Command::move_south);
  if (line == "BOND:LEFT") return append(Command::move_west);
  if (line == "BOND:RIGHT") return append(Command::move_east);
  if (line == "BOND:HARVEST") return append(Command::harvest);
  if (line == "BOND:DEPOSIT") return append(Command::deposit);
  return false;
}
}

AutomationCommandParseResult parse_automation_commands(std::string_view output, std::size_t maximum_commands) {
  AutomationCommandParseResult result;
  std::size_t begin{};
  while (begin < output.size()) {
    const auto end = output.find('\n', begin);
    auto line = output.substr(begin, end == std::string_view::npos ? output.size() - begin : end - begin);
    if (!line.empty() && line.back() == '\r') line.remove_suffix(1);
    if (!line.empty()) {
      if (result.commands.size() == maximum_commands) { result.limit_reached = true; break; }
      if (!append_command(line, result.commands)) ++result.ignored_lines;
    }
    if (end == std::string_view::npos) break;
    begin = end + 1;
  }
  return result;
}

} // namespace bond
