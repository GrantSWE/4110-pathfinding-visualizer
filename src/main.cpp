#include "grid.h"
#include "pathfinding.h"

#include <array>
#include <iomanip>
#include <iostream>
#include <string_view>

namespace {

struct DemoAlgorithm {
    std::string_view name;
    PathfindingAlgorithm algorithm;
};

void printResult(std::string_view name, const PathfindingResult& result) {
    std::cout << name << '\n'
              << "  Path found: " << (result.found ? "yes" : "no") << '\n'
              << "  Explored nodes: " << result.exploredNodes << '\n'
              << "  Path length: " << result.pathLength << '\n'
              << "  Execution time: " << std::fixed << std::setprecision(4)
              << result.executionTimeMilliseconds << " ms\n\n";
}

}  // namespace

int main() {
    Grid grid;
    const Position start{2, 2};
    const Position destination{17, 17};

    for (int row = 1; row < 18; ++row) {
        if (row != 9) {
            grid.setObstacle({row, 8});
        }
    }
    for (int column = 8; column < 19; ++column) {
        if (column != 14) {
            grid.setObstacle({13, column});
        }
    }

    constexpr std::array<DemoAlgorithm, 4> algorithms = {{
        {"Breadth-First Search", PathfindingAlgorithm::BreadthFirstSearch},
        {"Depth-First Search", PathfindingAlgorithm::DepthFirstSearch},
        {"Dijkstra's Algorithm", PathfindingAlgorithm::Dijkstra},
        {"A* Search", PathfindingAlgorithm::AStar},
    }};

    std::cout << "Pathfinding demo on a 20x20 grid\n"
              << "Start: (" << start.row << ", " << start.column << ")\n"
              << "Destination: (" << destination.row << ", " << destination.column << ")\n\n";

    for (const DemoAlgorithm& algorithm : algorithms) {
        printResult(
            algorithm.name,
            findPath(algorithm.algorithm, grid, start, destination));
    }

    return 0;
}

