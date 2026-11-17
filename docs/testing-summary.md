# Testing and Performance Summary

## Testing Environment

- **Computer:** MacBook Air (Mac15,12)
- **Processor:** Apple M3
- **Memory:** 16 GB
- **Architecture:** arm64
- **Operating system:** macOS 26.6.2 (build 25G83)
- **Compiler:** Apple Clang 21.0.0
- **CMake:** 4.5.0
- **Language standard:** C++17

All measurements in this document are native C++ results. The engine's `executionTimeMilliseconds` value measures the search and path reconstruction, not WebAssembly loading, browser rendering, or animation.

## Automated Correctness Results

The following required commands completed successfully from the repository root:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/pathfinding_tests
./build/pathfinding_demo
```

- CMake configured successfully and built `pathfinding_engine`, `pathfinding_demo`, and `pathfinding_tests`.
- CTest reported **1/1 registered test passed**.
- The direct test executable reported **9/9 scenarios passed**.
- The scenarios covered grid operations, boundaries, empty and obstructed grids, required detours, unreachable destinations, identical endpoints, invalid or blocked endpoints, and deterministic repeatability.
- The command-line demo found a path with every algorithm. Its single-run execution times were BFS `0.2818 ms`, DFS `0.0271 ms`, Dijkstra `0.3078 ms`, and A* `0.0638 ms`.

## Performance Method

The comparison reused the existing `pathfinding_engine` library and the same start `(2,2)` and destination `(17,17)` used by the native demo. The sample obstacle grid exactly matched `src/main.cpp`; the second scenario contained no obstacles.

Each algorithm received 10 warm-up runs followed by 100 measured runs per scenario. The table reports the arithmetic mean of the engine-provided execution times. Every measured repetition was also checked against a reference run for identical found status, explored-node count, path length, final path, and exploration order.

## Sample Grid with Obstacles

| Algorithm | Path found | Explored nodes | Path length | Average time (100 runs) |
| --- | --- | ---: | ---: | ---: |
| BFS | Yes | 363 | 30 | 0.235866 ms |
| DFS | Yes | 43 | 42 | 0.028975 ms |
| Dijkstra | Yes | 363 | 30 | 0.365690 ms |
| A* | Yes | 43 | 30 | 0.064012 ms |

## Empty 20×20 Grid

| Algorithm | Path found | Explored nodes | Path length | Average time (100 runs) |
| --- | --- | ---: | ---: | ---: |
| BFS | Yes | 388 | 30 | 0.169090 ms |
| DFS | Yes | 43 | 42 | 0.025733 ms |
| Dijkstra | Yes | 388 | 30 | 0.327475 ms |
| A* | Yes | 31 | 30 | 0.046300 ms |

## Observations

- **BFS** found a shortest 30-edge path in both scenarios, as expected for unit-cost movement. It explored most reachable cells before reaching the destination.
- **DFS** explored only 43 nodes in these particular scenarios and had the lowest average time, but its 42-edge path was longer. DFS does not guarantee a shortest path, and its apparent advantage depends strongly on deterministic neighbor order and endpoint placement.
- **Dijkstra's algorithm** produced the same shortest-path lengths and explored-node counts as BFS. With every movement cost equal to one, its priority queue adds overhead without providing a path-quality advantage over BFS.
- **A*** returned a shortest 30-edge path while exploring substantially fewer nodes, especially on the empty grid. Manhattan distance directs expansion toward the destination and is appropriate for four-directional unit-cost movement.

Under this project's movement rules, BFS, Dijkstra's algorithm, and A* guarantee shortest paths. DFS guarantees neither the shortest path nor consistently low exploration counts, although it returns a valid path when one is found.

## Measurement Limitations

- The grid contains only 400 cells, so all searches finish in fractions of a millisecond. Small differences can fluctuate because of timer precision, operating-system scheduling, cache state, and other system activity.
- These results were collected on one computer and should not be treated as hardware-independent benchmarks.
- The averages describe only two grid layouts and one start/destination pair. Different obstacle patterns can substantially change exploration order and node counts.
- The benchmark uses native C++ timing. Browser and WebAssembly timings may differ because they run in a different environment.
- Timing results show practical behavior for this project but do not replace asymptotic complexity analysis.
