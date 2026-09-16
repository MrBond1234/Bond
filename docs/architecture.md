# Architecture — v0.1

`Application` coordinates the product. Its presentation host is intentionally a CLI today; a Windows UI can replace that host without owning training rules.

| Boundary | Responsibility | v0.1 status |
|---|---|---|
| Application | composition, navigation, use-cases | skeleton |
| Simulation Engine | deterministic grid, robot commands and snapshots | working minimal core |
| Training/Lesson | campaign data and lesson lookup | PSV loader |
| Code Execution/Sandbox | compile/run learner code under resource limits | interface planned; no unsafe execution yet |
| Evaluation | inspect output/simulation results and score requirements | interface planned |
| UI | Windows desktop/editor, task and output panes | host seam only |
| Localization | translated keyed learner text | Thai/English PSV seed |
| Persistence | versioned progress save/load | working minimal core |
| Daily Generator | reproducible exercise from seed | working deterministic seed |

Data is versioned separately from C++ under `data/`. `campaign_v1.psv` is deliberately narrow for the bootstrap; later fields (starter code, validation rules, scoring, unlock conditions) should be additive to preserve loaders and saves.

The eventual execution service must run untrusted learner code in a Windows-restricted process/job object with time, memory, filesystem, and network limits. It must not execute arbitrary code in the application process.

`include/bond/execution.hpp` and `include/bond/evaluation.hpp` are intentionally dependency-inverted seams: the application can depend on contracts while the Windows sandbox and individual lesson evaluators arrive incrementally. `Localizer` similarly keeps Thai/English content out of UI code.

## UI target

The Windows UI should use one engineering-style workspace: simulation grid, code editor, task/requirements, and output/analysis. Keep simulation 2D and use animation only to explain an algorithm. A host interface can support WinUI 3 or Qt later without changing domain modules.
