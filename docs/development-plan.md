# Development Plan

Phase 1 established the repository structure, build configuration, and initial documentation. Phase 2 completed the native C++ pathfinding engine and correctness tests. WebAssembly and frontend work have not started.

## Phase 2: C++ Algorithms and Native Tests — Completed

Implemented BFS, DFS, Dijkstra's algorithm, and A* in C++17 with shared grid and result structures. Native correctness tests cover representative and edge-case grids.

## Phase 3: WebAssembly Integration

Compile the tested C++ code to WebAssembly with Emscripten. Expose a small interface for JavaScript and verify that browser results match native results.

## Phase 4: Interactive Frontend

Build the 20-by-20 grid, controls, obstacle editing, movable endpoints, exploration animation, path animation, and metric display with HTML, CSS, and vanilla JavaScript.

## Phase 5: Correctness and Performance Evaluation

Run the full correctness suite and compare all algorithms on consistent grid scenarios. Record execution time, explored nodes, path length, and notable exploration behavior.

## Phase 6: Final Deliverables

Complete the final report, capture screenshots, record the demonstration video, verify submission requirements, and prepare the repository for submission.
