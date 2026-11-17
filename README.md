# Pathfinding Algorithm Visualizer and Performance Analyzer

**Course:** CSCE 4110 — Algorithms

## Project Overview

This project is an interactive, browser-based tool for visualizing and comparing graph-search algorithms on a fixed 20-by-20 grid. Its pathfinding engine is written in C++17 and is designed for later compilation to WebAssembly with Emscripten. The future lightweight frontend will use HTML, CSS, and vanilla JavaScript to display each algorithm's exploration order, final path, and performance measurements.

The motivation is to make the behavior and tradeoffs of common pathfinding algorithms easier to understand. Running multiple algorithms on the same grid connects concepts such as completeness, optimality, uninformed search, and heuristic search to observable results.

> **Project status:** The native C++ engine and tests are complete. Phase 3 integration files now provide Emscripten bindings, a WebAssembly build script, and a minimal browser test page. The WebAssembly build still requires local verification after Emscripten is installed and activated. The full interactive frontend remains future work.

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
- Emscripten Embind interface around the existing C++ engine
- Reproducible WebAssembly build script
- Minimal browser integration and native-parity test page

### Planned

- Local WebAssembly build verification with an activated Emscripten SDK
- Manual browser verification of the generated module
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
- **WebAssembly toolchain:** Emscripten
- **Future frontend:** HTML5, CSS3, and vanilla JavaScript
- **Version control:** Git and GitHub

No frontend framework, backend server, database, or Node.js build system is used.

## Architecture

1. `Grid` owns obstacle state and provides validated grid operations without user-interface dependencies.
2. The pathfinding module accepts a shared grid and endpoints, then returns a `PathfindingResult` containing search status, exploration order, path, and metrics.
3. The native demo exercises the engine from a command-line program.
4. Native C++ tests verify grid behavior and algorithm correctness independently of any browser.
5. The Emscripten binding layer exposes the existing engine to JavaScript without moving pathfinding logic into the frontend.

## Repository Structure

```text
.
├── CMakeLists.txt
├── README.md
├── docs/
│   ├── development-plan.md
│   └── screenshots/
├── src/
│   ├── grid.cpp
│   ├── grid.h
│   ├── main.cpp
│   ├── pathfinding.cpp
│   ├── pathfinding.h
│   └── wasm_bindings.cpp
├── tests/
│   └── test_pathfinding.cpp
├── scripts/
│   └── build_wasm.sh
└── web/
    └── wasm-test.html      # Minimal integration test, not the final frontend
```

## Development Prerequisites

- A compiler with C++17 support, such as Apple Clang, Clang, GCC, or MSVC
- CMake 3.16 or newer
- A terminal for building and running the native targets
- Emscripten SDK and a modern WebAssembly-capable browser for Phase 3

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

## Emscripten Setup on macOS

Emscripten is not installed automatically by this project. Follow the [official Emscripten SDK instructions](https://emscripten.org/docs/getting_started/downloads.html). One standard installation sequence is:

```sh
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
emcc --version
```

Run `source /absolute/path/to/emsdk/emsdk_env.sh` in each new terminal before building, or add that command to the appropriate shell startup file after reviewing the SDK instructions. Emscripten was unavailable in the development environment when the Phase 3 files were prepared, so the generated artifacts and browser execution still require verification on a machine with an activated SDK.

## JavaScript Binding Contract

The generated module exposes:

```js
module.runPathfinding(algorithmId, startIndex, destinationIndex, blockedCellIndices)
```

- `algorithmId`: `0` for BFS, `1` for DFS, `2` for Dijkstra, or `3` for A*.
- Cell values are row-major indices from `0` through `399`, calculated as `row * 20 + column`.
- `blockedCellIndices` is a `module.IntVector` populated with zero or more cell indices.
- Invalid algorithms, out-of-range indices, and blocked endpoints return `validInput: false` with an `errorMessage` and do not run a search.
- Valid results include `found`, `explorationOrder`, `path`, `pathLength`, `exploredNodes`, and `executionTimeMilliseconds`.

`explorationOrder` and `path` are also row-major cell indices. Embind vector wrappers should be released with `.delete()` after their contents are copied.

## WebAssembly Build

After activating Emscripten, run from the repository root:

```sh
./scripts/build_wasm.sh
```

The script compiles `grid.cpp`, `pathfinding.cpp`, and `wasm_bindings.cpp` into:

```text
dist/wasm/pathfinding.js
dist/wasm/pathfinding.wasm
```

The entire `dist/` directory is intentionally excluded from Git because these files are generated build artifacts.

## Browser Integration Test

Browsers should load WebAssembly over HTTP rather than directly from `file://`. After a successful WebAssembly build, start a local static server from the repository root:

```sh
python3 -m http.server 8000
```

Then open:

```text
http://localhost:8000/web/wasm-test.html
```

The page runs all four C++ algorithms on the native demo grid, validates returned paths and unique exploration sequences, and compares path lengths and explored-node counts with the native demo. It prints a clear pass, failure, loading error, or runtime error. This page is only an integration test and is not the Phase 4 interactive frontend.

## Performance Metrics

- **Execution time:** Time spent inside the C++ search call, excluding future rendering and animation
- **Explored nodes:** Number of unique cells removed from the algorithm frontier and expanded
- **Path length:** Number of movement edges from start to destination; zero for an unreachable result or when both endpoints are the same

Execution times are intended for comparative observation rather than strict benchmarking. Native and future WebAssembly timings may differ because they use different execution environments.

## Future Frontend Work

After the prepared Phase 3 integration is built and manually verified, Phase 4 will add the interactive grid, controls, and animations. The current HTML page only exercises the WebAssembly API and displays raw test results.

## Acknowledgments

External references, tutorials, libraries, code examples, and assets used during development will be credited here and in the final report as appropriate. Any AI-assisted planning, documentation, code generation, debugging, or review will also be disclosed. AI-generated material will be reviewed, tested, and revised by the project author, who remains responsible for the final work and its correctness.
