# Bond Automation Training

A Windows-first C++20 automation-training simulator. Version 1.2 is a playable native Windows workbench with a visual harvesting grid, Thai-first 100-lesson campaign, isolated C++ execution worker, source-level evaluation, local progress, and a final-project guide.

## Status

The workbench uses native Win32 controls to keep the dependency footprint small. It is intentionally an engineering training workspace rather than a game skin: select a lesson, navigate the robot on the grid, harvest and deposit the target, then run/check C++ automation. The executable resolves its data and worker beside itself, so the built or installed app is self-contained.

## Prerequisites

- CMake 3.24 or newer (verified with 4.4.3)
- Visual Studio Build Tools with the **Desktop development with C++** workload
- Windows 10 version 19045 or newer (verified on Windows 10 19045 with MSVC 19.51)

Build and test from a Visual Studio Developer PowerShell:

```powershell
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
.\build\Debug\bond_training.exe
```

After the build completes, double-click `build\Debug\bond_training.exe` to play. CMake copies `data\` beside the executable automatically.

## Play a lesson

1. Start at Lesson 1. The grid uses **R** for the robot, **C** for a crop, and **D** for the depot.
2. Use the on-screen direction buttons to move; use **Harvest** while standing on a crop and **Deposit** while standing on the depot. The first route is Up, Right, Harvest, Left, Left, Deposit.
3. Write C++ in the editor and choose **Run C++ code**. Learner code runs only in the separate worker. It may return bounded simulator commands through standard output:

   ```cpp
   std::cout << "BOND:UP\n";
   std::cout << "BOND:RIGHT\n";
   std::cout << "BOND:HARVEST\n";
   std::cout << "BOND:LEFT\n";
   std::cout << "BOND:DEPOSIT\n";
   ```

   Supported commands are `BOND:UP`, `BOND:DOWN`, `BOND:LEFT`, `BOND:RIGHT`, `BOND:HARVEST`, and `BOND:DEPOSIT`.
4. When both the simulation target and lesson source requirement are satisfied, choose **Check and save**. Progress is stored per user under `%LOCALAPPDATA%\BondAutomationTraining`.

The worker detects a supported local Visual Studio Build Tools installation to prepare `cl.exe` when the app was launched from Explorer. If none is installed, the app remains playable manually and reports the missing compiler clearly.

## Create a click-to-play package

```powershell
cmake -S . -B build-release -DBUILD_TESTING=ON
cmake --build build-release --config Release
cmake --install build-release --config Release --prefix playable
ctest --test-dir build-release -C Release --output-on-failure
cpack --config build-release/CPackConfig.cmake -C Release
```

Open `playable\bond_training.exe`. The generated ZIP is a portable package; an installer/desktop shortcut is intentionally not created yet.

## Layout

- `data/` — lessons and Thai/English learner text
- `docs/` — architecture and 100-stage curriculum
- `include/bond/` and `src/` — C++ modules
- `tests/` — core behavior tests

See `docs/architecture.md` for the runtime and execution security boundary, and `docs/final_project.md` for the capstone deliverable.
