# Bond Automation Training

A Windows-first C++20 automation-training simulator. Version 0.5 supplies a native engineering workbench, deterministic multi-row simulation, a Thai-first campaign through lesson 50, source-level evaluation, and a separate execution worker.

## Status

The workbench uses native Win32 controls to keep the dependency footprint small: simulation, C++ editor, task/requirements, output/analysis, and Run/Step/Reset controls. The UI is intentionally an IDE/PLC-style training workspace rather than a game.

## Prerequisites

- CMake 3.24 or newer (verified with 4.4.3)
- Visual Studio Build Tools with the **Desktop development with C++** workload
- Windows 10 version 19045 or newer (verified on Windows 10 19045 with MSVC 19.51)

Open a Visual Studio Developer PowerShell, then run:

```powershell
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
.\build\Debug\bond_training.exe
```

## Layout

- `data/` — lessons and Thai/English learner text
- `docs/` — architecture and 100-stage curriculum
- `include/bond/` and `src/` — C++ modules
- `tests/` — core behavior tests

See `docs/architecture.md` for the security boundary around future code execution.
