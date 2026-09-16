# Architecture — v1.2

`Application` coordinates the product. The native Win32 presentation host owns only controls, rendering, and event routing; training rules remain in the core boundaries.

| Boundary | Responsibility | v0.1 status |
|---|---|---|
| Application | composition, navigation, use-cases | skeleton |
| Simulation Engine | deterministic grid, robot commands and snapshots | working minimal core |
| Training/Lesson | campaign data and lesson lookup | PSV loader |
| Code Execution/Sandbox | compile/run learner code under resource limits | Windows worker process and Job Object limits |
| Evaluation | inspect output/simulation results and score requirements | data-driven construct and simulation evaluator |
| UI | Windows desktop/editor, task, grid, output, and lesson controls | native Win32 playable workbench |
| Localization | translated keyed learner text | Thai/English PSV seed |
| Persistence | versioned progress save/load | working minimal core |
| Daily Generator | reproducible exercise from seed | working deterministic seed |

Data is versioned separately from C++ under `data/`. `campaign_v1.psv` is deliberately narrow for the bootstrap; later fields (starter code, validation rules, scoring, unlock conditions) should be additive to preserve loaders and saves.

`WorkerCodeExecutor` validates bounded source, writes a request file, and invokes `bond_execution_worker.exe` with `CreateProcessW`; no shell command is built from learner input. The worker compiles in a private temporary directory and starts the learner executable suspended inside a Job Object with a hard timeout, single-child process limit, memory cap, and kill-on-close. When `cl.exe` is not inherited from a developer shell, the worker can bootstrap a supported local Visual Studio environment through its fixed `VsDevCmd.bat` location; learner source remains a file and is never interpolated into that setup command. Compiler diagnostics and standard output are returned to the workbench. Learner code never runs in the main application process.

This is a training isolation boundary, not yet a hardened multi-tenant sandbox: restricted-token/AppContainer, explicit filesystem ACLs, network denial, asynchronous pipe draining, compiler allow-list configuration, and malware scanning remain required before exposing it outside a trusted managed-training machine.

`include/bond/execution.hpp`, `include/bond/automation_protocol.hpp`, and `include/bond/evaluation.hpp` are intentionally dependency-inverted seams. The bounded automation protocol accepts only six exact worker-output lines and maps them to simulator commands; arbitrary worker output is ignored. The evaluator understands `function:<name>` campaign requirements, which require a matching callable identifier, and `all:<item>,<item>` requirements, which require every listed construct before simulation scoring. It ignores comments and quoted strings before inspecting source, but remains a deliberately small source inspection rather than a compiler or AST parser. `Localizer` similarly keeps Thai/English content out of UI code.

## UI target

The simulation uses a compact deterministic single-row layout for early lessons and a multi-row layout with blocked cells for five- and six-crop loop lessons. Snapshots expose successful action count and remaining crops so the UI and evaluator can report route progress without parsing UI state.

The native Win32 workbench uses one engineering-style workspace: task/requirements, a painted simulation grid, six direct simulator controls, lesson navigation, a C++ editor, output/analysis, and a Check-and-save completion action. It resolves `data/` and the worker beside the executable first; CMake copies this layout for developer builds and emits an installable portable ZIP without adding a large UI framework.
