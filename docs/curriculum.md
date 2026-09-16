# Curriculum: 100-stage campaign

Every stage is 1% curriculum completion; 100% reports completion of this curriculum, not complete C++ mastery. Modules contain ten stages each. v0.1 ships the data/runtime foundation for stages 1–10.

| Module | Stages | Focus |
|---|---:|---|
| 1. C++ Fundamentals | 1–10 | program structure, variables, types, I/O, arithmetic, conditions, loops, functions, debugging |
| 2. Conditions & Control Logic | 11–20 | `if`, `else`, `switch`, Boolean logic, conditions, module assessment |
| 3. Loops & Iteration | 21–30 | `for`, `while`, `do`/`while`, nested loops, iteration control, module assessment |
| 4. Functions & Modular Automation | 31–40 | declarations, parameters, return values, decomposition, reusable route helpers, module assessment |
| 5. STL | 41–50 | vector, string, map, set, iterators, algorithms, module assessment |
| 6. Memory & Resources | 51–60 | stack/heap, ownership, move semantics, smart pointers, resource safety, profiling basics |
| 7. Templates & Algorithms | 61–70 | function/class templates, concepts introduction, sorting, searching, complexity |
| 8. Data Structures | 71–80 | queues, stacks, priority queues, graphs, representation trade-offs, module assessment |
| 9. Pathfinding & Optimization | 81–90 | BFS, Dijkstra, A*, heuristics, scheduling, optimization measurements |
| 10. Software Architecture | 91–100 | interfaces, separation, testing, persistence, localization, final automation project |

## v0.4 playable sequence (1–40)

1. Start a program and issue one harvest command.
2. Store a movement count in an integer.
3. Use arithmetic to calculate harvest capacity.
4. Branch when a crop is present.
5. Repeat a fixed harvest route with `for`.
6. Continue until the depot condition is met with `while`.
7. Extract a route into a function.
8. Pass required harvest quantity as a parameter.
9. Return a status value and report failure.
10. Combine fundamentals in a mini automated harvest cycle; this is Module 1 assessment.
11. Use `if` to approve a harvest route.
12. Use `else` for a safe fallback route.
13. Combine safety conditions with `&&`.
14. Store and use a Boolean readiness status.
15. Select a route mode with `switch`.
16. Handle station modes with `case`.
17. Use `!` to stop an unsafe route.
18. Inspect crop and depot state with conditions.
19. Route fault recovery with `switch`.
20. Combine conditional and switch logic in the Module 2 assessment.
21. Repeat a fixed multi-row harvest route with `for`.
22. Continue a route until the inventory condition is met with `while`.
23. Use `do`/`while` for an inspection that must run once.
24. Sweep a two-dimensional field with nested `for` loops.
25. Track each route iteration while harvesting a row.
26. Use `continue` to skip a blocked inspection point.
27. Use `while` to process an extended six-crop route.
28. Combine nested loops and a condition for a multi-row route.
29. Use `break` to stop a loop on a safety condition.
30. Complete the Module 3 loop-and-iteration automation assessment.
31. Declare a helper that moves the robot toward a crop.
32. Extract a row harvest into a reusable function.
33. Extract depot delivery into a separate function.
34. Pass route readiness into a helper parameter.
35. Return a route status to the calling automation.
36. Break an extended harvest into named route segments.
37. Log route status through a dedicated helper.
38. Combine a safety decision with a reusable route function.
39. Verify delivery through a helper that reports success.
40. Complete the Module 4 functions-and-modular-automation assessment.
