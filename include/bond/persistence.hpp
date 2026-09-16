#pragma once
#include "bond/domain.hpp"
#include <filesystem>
namespace bond { class ProgressStore { public: static bool save(const std::filesystem::path&, const Progress&); static Progress load(const std::filesystem::path&); }; }
