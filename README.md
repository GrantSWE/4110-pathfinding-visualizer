# Pathfinding Algorithm Visualizer and Performance Analyzer

**Course:** CSCE 4110 — Algorithms

## Project Overview

This project will be an interactive, browser-based tool for visualizing and comparing graph-search algorithms on a fixed 20-by-20 grid. The pathfinding logic will be written in C++17 and compiled to WebAssembly with Emscripten. A lightweight frontend built with HTML, CSS, and vanilla JavaScript will display each algorithm's exploration order, final path, and performance measurements.

The motivation is to make the behavior and tradeoffs of common pathfinding algorithms easier to understand. Watching the same grid solved by multiple algorithms can connect theoretical concepts such as completeness, optimality, uninformed search, and heuristic search to observable behavior.

> **Project status:** Phase 1 repository setup and planning. Algorithms, WebAssembly integration, frontend behavior, and tests are not yet implemented.

## Planned Features

- Fixed 20-by-20 grid
- Four-directional movement: up, down, left, and right
- Equal movement cost for every traversable cell
- Adjustable start and destination positions
- Click-to-add and click-to-remove obstacles
- Animated exploration order and final path
- Selection among BFS, DFS, Dijkstra's algorithm, and A* search
- Execution time, explored-node count, and path-length metrics
- Correctness tests and side-by-side algorithm comparisons

## Planned Algorithms

### Breadth-First Search (BFS)

BFS explores the grid level by level using a queue. With equal movement costs, it is complete and finds a shortest path in number of grid steps when a path exists.

### Depth-First Search (DFS)

DFS follows one branch as deeply as possible before backtracking, typically using a stack or recursion. It can demonstrate a very different exploration order from BFS, but it does not guarantee a shortest path.

### Dijkstra's Algorithm

Dijkstra's algorithm repeatedly expands the unvisited node with the smallest known distance from the start. It guarantees a shortest path for nonnegative edge weights. Because this project's traversable cells have equal cost, its result should match BFS's shortest-path length, although its implementation and exploration behavior will differ.

### A* Search

A* prioritizes nodes using the known distance from the start plus an estimated distance to the destination. This project will use Manhattan distance, which matches four-directional grid movement and does not overestimate the remaining path cost. A* should find a shortest path while often exploring fewer nodes than uninformed searches.

## Technology Stack

- **Algorithm implementation:** C++17
- **Build configuration:** CMake and CTest
- **WebAssembly toolchain:** Emscripten
- **Frontend:** HTML5, CSS3, and vanilla JavaScript
- **Runtime environment:** Modern desktop web browser
- **Version control:** Git and GitHub

No frontend framework, backend server, database, or Node.js build system is planned.

## Proposed Architecture

The project will keep the algorithm logic separate from presentation concerns:

1. The C++ layer will represent the grid, run a selected algorithm, and return structured results including exploration order, final path, and metrics.
2. Emscripten bindings will expose a small C++ interface to JavaScript through WebAssembly.
3. JavaScript will manage user input, call the WebAssembly module, and schedule animations.
4. HTML and CSS will define the controls, grid, metric display, and visual states.
5. Native C++ tests will verify algorithm correctness independently of the browser interface.

This separation will allow the algorithms to be tested with a native compiler before WebAssembly integration.

## Repository Structure

```text
.
├── CMakeLists.txt          # C++17 and future CTest configuration
├── README.md               # Project overview and setup notes
├── docs/
│   ├── development-plan.md # Planned project phases
│   └── screenshots/        # Images for documentation and the final report
├── scripts/                # Future build and utility scripts
├── src/                    # Future C++ source and header files
├── tests/                  # Future native correctness tests
└── web/                    # Future HTML, CSS, JavaScript, and web assets
```

Empty project directories contain `.gitkeep` files so Git can preserve the planned structure before implementation begins.

## Development Prerequisites

The following tools will be required in later phases:

- A compiler with C++17 support, such as Clang, GCC, or MSVC
- CMake 3.16 or newer
- Emscripten SDK (`emcc`) for WebAssembly builds
- A modern browser with WebAssembly support
- Git for version control

## Build and Execution

> **Pending implementation and verification.** There are currently no application or test targets because source files have not been created. The final native and WebAssembly commands will be documented only after they have been implemented and tested.

The intended workflows are:

- Configure and build native C++ targets with CMake.
- Run native correctness tests through CTest.
- Configure Emscripten through its CMake toolchain and generate browser-ready WebAssembly output.
- Serve the `web/` content from a local HTTP server and open it in a browser.

Exact commands are intentionally deferred to avoid documenting an unverified build process.

## Planned Testing and Evaluation

Correctness testing will begin with native C++ tests and will cover:

- Empty grids with direct paths
- Grids with obstacles and required detours
- Unreachable destinations
- Start and destination positions at boundaries and corners
- Start equal to destination
- Path validity, including adjacent moves and obstacle avoidance
- Expected shortest-path lengths for BFS, Dijkstra's algorithm, and A*
- Valid, though not necessarily shortest, DFS paths
- Consistent results between native and WebAssembly execution

Algorithm evaluation will run each implementation on the same predefined and user-created grids. Results will be compared for correctness, exploration behavior, and measured performance. Timing tests will be repeated when practical to reduce the effect of one-time runtime noise.

## Performance Metrics

- **Execution time:** Time spent running the algorithm, measured separately from animation time when possible
- **Explored nodes:** Number of grid cells processed during the search
- **Path length:** Number of moves from the start cell to the destination cell; reported as unavailable when no path exists

The final analysis will explain that browser and native timings may not be directly comparable because WebAssembly and native builds use different execution environments.

## Acknowledgments

External references, tutorials, libraries, code examples, and assets used during development will be credited here and in the final report as appropriate. Any AI-assisted planning, documentation, code generation, debugging, or review will also be disclosed. AI-generated material will be reviewed, tested, and revised by the project author, who remains responsible for the final work and its correctness.
