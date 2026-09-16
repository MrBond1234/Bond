# Final Project — Route-Control Workbench

Lesson 100 is the capstone: deliver a small C++ route-control component for the simulated harvesting cell. The learner must describe the boundary, implement it, emit safe `BOND:` route commands through the isolated worker, and verify delivery on the workbench grid rather than in the workbench process.

## Minimum deliverable

1. Define a `RoutePlanner`-style class with at least one `virtual` operation.
2. Implement a concrete planner using `override`.
3. Use the simulator outcome to report successful harvest delivery and choose **Check and save** in the workbench.
4. Keep learner-facing messages behind localization keys in product code.
5. Persist completed lesson state through `ProgressStore`.
6. Provide at least one assertion or automated test for a route invariant.

## Review rubric

| Area | Evidence |
|---|---|
| Architecture | responsibility is isolated behind a class boundary |
| Safety | learner code is executed only by the worker process |
| Correctness | target harvest is delivered to the depot |
| Maintainability | localized text and persistence boundaries are preserved |
| Verification | the learner supplies a repeatable test or assertion |

Completion means the curriculum has been completed, not that the learner has mastered every C++ production concern. The workbench's source evaluator is a training aid; worker compilation and manual review remain part of final-project assessment.
