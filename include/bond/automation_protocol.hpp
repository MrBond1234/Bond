#pragma once

#include "bond/domain.hpp"
#include <cstddef>
#include <string_view>
#include <vector>

namespace bond {

struct AutomationCommandParseResult {
  std::vector<Command> commands;
  std::size_t ignored_lines{};
  bool limit_reached{};
};

// Learner programs communicate with the simulator only through these bounded worker-output lines:
// BOND:UP, BOND:DOWN, BOND:LEFT, BOND:RIGHT, BOND:HARVEST, and BOND:DEPOSIT.
[[nodiscard]] AutomationCommandParseResult parse_automation_commands(std::string_view output,
                                                                       std::size_t maximum_commands = 128);

} // namespace bond
