# Architecture — v0.7

`Application` coordinates the product. Its presentation host is intentionally a CLI today; a Windows UI can replace that host without owning training rules.

| Boundary | Responsibility | v0.1 status |
|---|---|---|
| Application | composition, navigation, use-cases | skeleton |
| Simulation Engine | deterministic grid, robot commands and snapshots | working minimal core |
| Training/Lesson | campaign data and lesson lookup | PSV loader |
| Code Execution/Sandbox | compile/run learner code under resource limits | Windows worker process and Job Object limits |
| Evaluation | inspect output/simulation results and score requirements | data-driven construct and simulation evaluator |
| UI | Windows desktop/editor, task and output panes | native Win32 workbench |
| Localization | translated keyed learner text | Thai/English PSV seed |
| Persistence | versioned progress save/load | working minimal core |
| Daily Generator | reproducible exercise from seed | working deterministic seed |

Data is versioned separately from C++ under `data/`. `campaign_v1.psv` is deliberately narrow for the bootstrap; later fields (starter code, validation rules, scoring, unlock conditions) should be additive to preserve loaders and saves.

`WorkerCodeExecutor` validates bounded source, writes a request file, and invokes `bond_execution_worker.exe` with `CreateProcessW`; no shell command is built from learner input. The worker compiles in a private temporary directory and starts the learner executable suspended inside a Job Object with a hard timeout, single-child process limit, memory cap, and kill-on-close. Compiler diagnostics and standard output are returned to the workbench. Learner code never runs in the main application process.

This is a training isolation boundary, not yet a hardened multi-tenant sandbox: restricted-token/AppContainer, explicit filesystem ACLs, network denial, asynchronous pipe draining, compiler allow-list configuration, and malware scanning remain required before exposing it outside a trusted managed-training machine.

`include/bond/execution.hpp` and `include/bond/evaluation.hpp` are intentionally dependency-inverted seams: the application can depend on contracts while the Windows sandbox and individual lesson evaluators arrive incrementally. The evaluator understands `function:<name>` campaign requirements, which require a matching callable identifier, and `all:<item>,<item>` requirements, which require every listed construct before simulation scoring. It remains a deliberately small source inspection rather than a compiler or AST parser. `Localizer` similarly keeps Thai/English content out of UI code.

## UI target

The simulation uses a compact deterministic single-row layout for early lessons and a multi-row layout with blocked cells for five- and six-crop loop lessons. Snapshots expose successful action count and remaining crops so the UI and evaluator can report route progress without parsing UI state.

The native Win32 workbench uses one engineering-style workspace: simulation grid, code editor, task/requirements, output/analysis, and Run/Step/Reset controls. It adds no large UI framework; a future WinUI 3 or Qt host can consume the same core boundaries.
