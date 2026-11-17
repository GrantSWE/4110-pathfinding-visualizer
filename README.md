# Pathfinding Algorithm Visualizer and Performance Analyzer

**Course:** CSCE 4110 — Algorithms

## Project Overview

This project is an interactive, browser-based tool for visualizing and comparing graph-search algorithms on a fixed 20-by-20 grid. Its pathfinding engine is written in C++17 and is designed for later compilation to WebAssembly with Emscripten. The future lightweight frontend will use HTML, CSS, and vanilla JavaScript to display each algorithm's exploration order, final path, and performance measurements.

The motivation is to make the behavior and tradeoffs of common pathfinding algorithms easier to understand. Running multiple algorithms on the same grid connects concepts such as completeness, optimality, uninformed search, and heuristic search to observable results.

> **Project status:** Phase 2 is implemented. The native C++ pathfinding engine, command-line demo, and automated correctness tests are complete. WebAssembly integration and frontend functionality remain future work.

## Functionality

### Completed

- Fixed 20-by-20 grid with coordinate and index conversion
- Four-directional movement with equal movement cost
- Obstacle placement, removal, and bounds checking
- Configurable start and destination coordinates
- BFS, DFS, Dijkstra's algorithm, and A* search
- Exploration order and reconstructed path results
- Execution time, explored-node count, and path-length metrics
- Native command-line demonstration
- Dependency-free native correctness and repeatability tests

### Planned

- C++ compilation to WebAssembly with Emscripten
- JavaScript-to-WebAssembly integration
- Interactive obstacle and endpoint editing
- Animated exploration order and final path
- Browser-based controls and algorithm comparison display

## Implemented Algorithms

### Breadth-First Search (BFS)

BFS uses `std::queue` to explore the grid level by level. It marks cells when they enter the queue, preventing duplicate work. With equal movement costs, BFS is complete and returns a shortest path when one exists.

### Depth-First Search (DFS)

DFS uses an explicit `std::stack` and follows one branch before backtracking. Neighbors are pushed in reverse order so expansion follows the grid's documented deterministic order. DFS returns a valid path when reachable but does not guarantee the shortest path.

### Dijkstra's Algorithm

Dijkstra's algorithm uses `std::priority_queue` to expand the cell with the smallest known distance from the start. Grid index provides deterministic tie-breaking. It guarantees a shortest path because all movement costs are nonnegative; on this equal-cost grid, its path length matches BFS.

### A* Search

A* uses `std::priority_queue` and ranks cells by distance from the start plus Manhattan distance to the destination. Additional cost and grid-index fields provide deterministic tie-breaking. Manhattan distance is admissible for four-directional unit-cost movement, so A* returns a shortest path.

All algorithms use the same `Grid`, validate endpoints before searching, record each expanded cell once, and reconstruct successful paths through parent indices.

## Data Structures and Complexity

The grid stores 400 obstacle flags in `std::array<bool, 400>`. Coordinates are converted to row-major indices for constant-time state access. Traversable neighbors are returned in deterministic **up, right, down, left** order.

Let `V` be the number of grid cells and `E` the number of valid neighbor connections. For this project, `V` is always 400 and each cell has at most four outgoing connections.

| Algorithm | Primary structure | Time complexity | Auxiliary space |
| --- | --- | --- | --- |
| BFS | `std::queue` | `O(V + E)` | `O(V)` |
| DFS | `std::stack` | `O(V + E)` | `O(V)` |
| Dijkstra | `std::priority_queue` | `O((V + E) log V)` | `O(V + E)` worst case |
| A* | `std::priority_queue` | `O((V + E) log V)` worst case | `O(V + E)` worst case |

The fixed grid keeps all practical memory use small. A* may explore fewer cells than uninformed algorithms, but its worst case remains similar to Dijkstra's algorithm.

## Technology Stack

- **Algorithm implementation:** C++17
- **Build and test configuration:** CMake and CTest
- **Future WebAssembly toolchain:** Emscripten
- **Future frontend:** HTML5, CSS3, and vanilla JavaScript
- **Version control:** Git and GitHub

No frontend framework, backend server, database, or Node.js build system is used.

## Architecture

1. `Grid` owns obstacle state and provides validated grid operations without user-interface dependencies.
2. The pathfinding module accepts a shared grid and endpoints, then returns a `PathfindingResult` containing search status, exploration order, path, and metrics.
3. The native demo exercises the engine from a command-line program.
4. Native C++ tests verify grid behavior and algorithm correctness independently of any browser.
5. A future Emscripten binding layer will expose the existing engine to JavaScript without moving pathfinding logic into the frontend.

## Repository Structure

```text
.
├── CMakeLists.txt
├── README.md
├── docs/
│   ├── development-plan.md
│   └── screenshots/
├── scripts/
├── src/
│   ├── grid.cpp
│   ├── grid.h
│   ├── main.cpp
│   ├── pathfinding.cpp
│   └── pathfinding.h
├── tests/
│   └── test_pathfinding.cpp
└── web/                    # Reserved for future frontend work
```

## Development Prerequisites

- A compiler with C++17 support, such as Apple Clang, Clang, GCC, or MSVC
- CMake 3.16 or newer
- A terminal for building and running the native targets
- Emscripten SDK and a modern WebAssembly-capable browser beginning in Phase 3

## Native Build

From the repository root, configure and build the project:

```sh
cmake -S . -B build
cmake --build build
```

The build creates the reusable `pathfinding_engine` library, `pathfinding_demo` executable, and `pathfinding_tests` executable.

## Command-Line Demo

After building, run:

```sh
./build/pathfinding_demo
```

The demo creates one obstacle grid, runs all four algorithms, and prints whether a path was found, explored-node count, path length, and C++ execution time in milliseconds.

## Automated Tests

Run the registered test through CTest:

```sh
ctest --test-dir build --output-on-failure
```

The tests cover empty and obstructed grids, required detours, unreachable destinations, identical endpoints, invalid or blocked endpoints, boundary routes, path validity, shortest-path agreement, duplicate exploration prevention, and deterministic repeatability. An independent BFS implementation in the test file provides reference shortest-path lengths.

The test executable can also be run directly for individual scenario output:

```sh
./build/pathfinding_tests
```

## Performance Metrics

- **Execution time:** Time spent inside the C++ search call, excluding future rendering and animation
- **Explored nodes:** Number of unique cells removed from the algorithm frontier and expanded
- **Path length:** Number of movement edges from start to destination; zero for an unreachable result or when both endpoints are the same

Execution times are intended for comparative observation rather than strict benchmarking. Native and future WebAssembly timings may differ because they use different execution environments.

## Future WebAssembly and Frontend Work

Phase 3 will add Emscripten compilation and a small JavaScript-facing binding layer. Phase 4 will add the interactive grid and animations. No WebAssembly bindings, HTML, CSS, or JavaScript functionality are implemented yet.

## Acknowledgments

External references, tutorials, libraries, code examples, and assets used during development will be credited here and in the final report as appropriate. Any AI-assisted planning, documentation, code generation, debugging, or review will also be disclosed. AI-generated material will be reviewed, tested, and revised by the project author, who remains responsible for the final work and its correctness.
