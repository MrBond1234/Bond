# Repository working agreement

## Patch-first development

1. Inspect the existing implementation before changing it.
2. Identify the root cause and patch only the necessary area of the existing file.
3. Do not create replacement copies such as `_new`, `_fixed`, `_v2`, or `_final`.
4. Do not rewrite a subsystem unless the task genuinely requires it.
5. Build after every change and run the relevant tests. Diagnose failures and patch again.
6. Use Git commits as version history, not duplicate files.
7. Preserve backward compatibility for save files and content/config formats where practical.

## Project conventions

- Primary source language: modern C++ (C++20); Windows is the primary platform.
- Thai is the default learner-facing language. Keep compiler diagnostics available verbatim.
- Keep lessons, requirements, scoring, and localized text data-driven under `data/` where feasible.
- Maintain boundaries between Application, Simulation, Training, Execution, Evaluation, UI, Localization, Persistence, and Daily Exercise modules.
- Prefer small, testable units and add tests alongside behavioral changes.
