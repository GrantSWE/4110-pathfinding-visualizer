# Development Plan

Phase 1 establishes the repository structure, build configuration, and initial documentation. No algorithm or frontend implementation is included in this phase.

## Phase 2: C++ Algorithms and Native Tests

Implement BFS, DFS, Dijkstra's algorithm, and A* in C++17. Define shared grid and result structures, then add native correctness tests for representative and edge-case grids.

## Phase 3: WebAssembly Integration

Compile the tested C++ code to WebAssembly with Emscripten. Expose a small interface for JavaScript and verify that browser results match native results.

## Phase 4: Interactive Frontend

Build the 20-by-20 grid, controls, obstacle editing, movable endpoints, exploration animation, path animation, and metric display with HTML, CSS, and vanilla JavaScript.

## Phase 5: Correctness and Performance Evaluation

Run the full correctness suite and compare all algorithms on consistent grid scenarios. Record execution time, explored nodes, path length, and notable exploration behavior.

## Phase 6: Final Deliverables

Complete the final report, capture screenshots, record the demonstration video, verify submission requirements, and prepare the repository for submission.

