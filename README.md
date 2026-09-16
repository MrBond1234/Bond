# Bond Automation Training

A Windows-first C++20 learning foundation where learners automate a small harvesting robot. Version 0.1 supplies the campaign, deterministic simulation, progress format, daily-exercise seed, Thai/English lesson text, and extension points for secure execution and evaluation.

## Status

This is a developer foundation, not yet a complete learner application. It deliberately does **not** compile or run untrusted learner code until a Windows sandbox is implemented.

## Prerequisites

- CMake 3.24 or newer
- Visual Studio Build Tools with the **Desktop development with C++** workload

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
