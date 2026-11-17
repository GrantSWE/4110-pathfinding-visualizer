#include "grid.h"
#include "pathfinding.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <exception>
#include <functional>
#include <iostream>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

#define REQUIRE(condition) require((condition), #condition, __FILE__, __LINE__)

using SearchFunction = PathfindingResult (*)(const Grid&, Position, Position);

struct AlgorithmCase {
    std::string_view name;
    SearchFunction search;
    bool mustFindShortestPath;
};

constexpr std::array<AlgorithmCase, 4> kAlgorithms = {{
    {"BFS", breadthFirstSearch, true},
    {"DFS", depthFirstSearch, false},
    {"Dijkstra", dijkstraSearch, true},
    {"A*", aStarSearch, true},
}};

void require(bool condition, const char* expression, const char* file, int line) {
    if (!condition) {
        std::ostringstream message;
        message << file << ':' << line << ": requirement failed: " << expression;
        throw std::runtime_error(message.str());
    }
}

std::size_t referenceShortestPathLength(
    const Grid& grid,
    Position start,
    Position destination) {
    if (!grid.hasValidEndpoints(start, destination)) {
        return Grid::kCellCount;
    }

    std::array<int, Grid::kCellCount> distance;
    distance.fill(-1);
    std::queue<Position> frontier;

    distance[grid.toIndex(start)] = 0;
    frontier.push(start);

    constexpr std::array<Position, 4> offsets = {
        Position{-1, 0},
        Position{1, 0},
        Position{0, -1},
        Position{0, 1},
    };

    while (!frontier.empty()) {
        const Position current = frontier.front();
        frontier.pop();

        if (current == destination) {
            return static_cast<std::size_t>(distance[grid.toIndex(current)]);
        }

        for (const Position offset : offsets) {
            const Position next{current.row + offset.row, current.column + offset.column};
            if (!grid.isTraversable(next)) {
                continue;
            }

            const std::size_t nextIndex = grid.toIndex(next);
            if (distance[nextIndex] == -1) {
                distance[nextIndex] = distance[grid.toIndex(current)] + 1;
                frontier.push(next);
            }
        }
    }

    return Grid::kCellCount;
}

void verifyResult(
    const Grid& grid,
    Position start,
    Position destination,
    const PathfindingResult& result) {
    REQUIRE(result.exploredNodes == result.explorationOrder.size());
    REQUIRE(result.executionTimeMilliseconds >= 0.0);

    std::array<bool, Grid::kCellCount> explored{};
    for (const Position position : result.explorationOrder) {
        REQUIRE(grid.isTraversable(position));
        const std::size_t index = grid.toIndex(position);
        REQUIRE(!explored[index]);
        explored[index] = true;
    }

    if (!result.found) {
        REQUIRE(result.path.empty());
        REQUIRE(result.pathLength == 0);
        return;
    }

    REQUIRE(!result.path.empty());
    REQUIRE(result.path.front() == start);
    REQUIRE(result.path.back() == destination);
    REQUIRE(result.pathLength == result.path.size() - 1);

    for (std::size_t index = 0; index < result.path.size(); ++index) {
        REQUIRE(grid.isTraversable(result.path[index]));
        if (index > 0) {
            const Position previous = result.path[index - 1];
            const Position current = result.path[index];
            REQUIRE(std::abs(previous.row - current.row) +
                        std::abs(previous.column - current.column) ==
                    1);
        }
    }
}

void verifyAllAlgorithms(
    const Grid& grid,
    Position start,
    Position destination,
    bool expectedFound) {
    const std::size_t referenceLength = referenceShortestPathLength(grid, start, destination);

    for (const AlgorithmCase& algorithm : kAlgorithms) {
        const PathfindingResult result = algorithm.search(grid, start, destination);
        verifyResult(grid, start, destination, result);
        REQUIRE(result.found == expectedFound);
        if (expectedFound && algorithm.mustFindShortestPath) {
            REQUIRE(result.pathLength == referenceLength);
        }
    }
}

void testGridOperationsAndBoundaries() {
    Grid grid;

    REQUIRE(grid.isInBounds({0, 0}));
    REQUIRE(grid.isInBounds({19, 19}));
    REQUIRE(!grid.isInBounds({-1, 0}));
    REQUIRE(!grid.isInBounds({20, 0}));
    REQUIRE(!grid.setObstacle({20, 20}));
    REQUIRE(grid.toIndex({0, 0}) == 0);
    REQUIRE(grid.toIndex({19, 19}) == 399);
    REQUIRE(grid.toPosition(0) == Position({0, 0}));
    REQUIRE(grid.toPosition(399) == Position({19, 19}));

    bool positionException = false;
    try {
        static_cast<void>(grid.toIndex({-1, 0}));
    } catch (const std::out_of_range&) {
        positionException = true;
    }
    REQUIRE(positionException);

    bool indexException = false;
    try {
        static_cast<void>(grid.toPosition(Grid::kCellCount));
    } catch (const std::out_of_range&) {
        indexException = true;
    }
    REQUIRE(indexException);

    const std::vector<Position> cornerNeighbors = grid.validNeighbors({0, 0});
    REQUIRE(cornerNeighbors.size() == 2);
    REQUIRE(cornerNeighbors[0] == Position({0, 1}));
    REQUIRE(cornerNeighbors[1] == Position({1, 0}));

    const std::vector<Position> centerNeighbors = grid.validNeighbors({5, 5});
    REQUIRE(centerNeighbors.size() == 4);
    REQUIRE(centerNeighbors[0] == Position({4, 5}));
    REQUIRE(centerNeighbors[1] == Position({5, 6}));
    REQUIRE(centerNeighbors[2] == Position({6, 5}));
    REQUIRE(centerNeighbors[3] == Position({5, 4}));

    REQUIRE(grid.setObstacle({0, 1}));
    REQUIRE(grid.isBlocked({0, 1}));
    REQUIRE(!grid.isTraversable({0, 1}));
    REQUIRE(grid.removeObstacle({0, 1}));
    REQUIRE(!grid.isBlocked({0, 1}));
    grid.setObstacle({1, 0});
    grid.clearObstacles();
    REQUIRE(grid.isTraversable({1, 0}));
}

void testEmptyGrid() {
    const Grid grid;
    const Position start{0, 0};
    const Position destination{19, 19};
    REQUIRE(referenceShortestPathLength(grid, start, destination) == 38);
    verifyAllAlgorithms(grid, start, destination, true);
}

void testObstacleGridAndDetour() {
    Grid grid;
    for (int row = 0; row <= 3; ++row) {
        grid.setObstacle({row, 2});
    }

    const Position start{0, 0};
    const Position destination{0, 4};
    REQUIRE(referenceShortestPathLength(grid, start, destination) == 12);
    verifyAllAlgorithms(grid, start, destination, true);
}

void testLongWallDetour() {
    Grid grid;
    for (int column = 0; column < 19; ++column) {
        grid.setObstacle({10, column});
    }

    const Position start{0, 0};
    const Position destination{19, 0};
    REQUIRE(referenceShortestPathLength(grid, start, destination) == 57);
    verifyAllAlgorithms(grid, start, destination, true);
}

void testUnreachableDestination() {
    Grid grid;
    const Position start{0, 0};
    const Position destination{19, 19};
    grid.setObstacle({18, 19});
    grid.setObstacle({19, 18});

    REQUIRE(referenceShortestPathLength(grid, start, destination) == Grid::kCellCount);
    verifyAllAlgorithms(grid, start, destination, false);
}

void testStartEqualsDestination() {
    const Grid grid;
    const Position endpoint{7, 11};

    for (const AlgorithmCase& algorithm : kAlgorithms) {
        const PathfindingResult result = algorithm.search(grid, endpoint, endpoint);
        verifyResult(grid, endpoint, endpoint, result);
        REQUIRE(result.found);
        REQUIRE(result.path.size() == 1);
        REQUIRE(result.pathLength == 0);
        REQUIRE(result.exploredNodes == 1);
    }
}

void testInvalidAndBlockedEndpoints() {
    Grid grid;
    const std::array<std::pair<Position, Position>, 2> invalidEndpoints = {{
        {{-1, 0}, {5, 5}},
        {{0, 0}, {20, 5}},
    }};

    for (const AlgorithmCase& algorithm : kAlgorithms) {
        for (const auto& endpoints : invalidEndpoints) {
            const PathfindingResult result =
                algorithm.search(grid, endpoints.first, endpoints.second);
            REQUIRE(!result.found);
            REQUIRE(result.path.empty());
            REQUIRE(result.explorationOrder.empty());
        }
    }

    grid.setObstacle({0, 0});
    verifyAllAlgorithms(grid, {0, 0}, {5, 5}, false);
    grid.removeObstacle({0, 0});
    grid.setObstacle({5, 5});
    verifyAllAlgorithms(grid, {0, 0}, {5, 5}, false);
}

void testBoundaryRoute() {
    Grid grid;
    for (int row = 0; row < Grid::kRows; ++row) {
        for (int column = 1; column < Grid::kColumns; ++column) {
            grid.setObstacle({row, column});
        }
    }

    verifyAllAlgorithms(grid, {0, 0}, {19, 0}, true);
}

void testRepeatabilityAndDispatcher() {
    Grid grid;
    for (int row = 2; row < 18; ++row) {
        if (row != 8) {
            grid.setObstacle({row, 9});
        }
    }
    const Position start{3, 3};
    const Position destination{16, 16};

    for (const AlgorithmCase& algorithm : kAlgorithms) {
        const PathfindingResult first = algorithm.search(grid, start, destination);
        const PathfindingResult second = algorithm.search(grid, start, destination);
        REQUIRE(first.found == second.found);
        REQUIRE(first.explorationOrder == second.explorationOrder);
        REQUIRE(first.path == second.path);
        REQUIRE(first.exploredNodes == second.exploredNodes);
        REQUIRE(first.pathLength == second.pathLength);
    }

    const PathfindingResult direct = breadthFirstSearch(grid, start, destination);
    const PathfindingResult dispatched = findPath(
        PathfindingAlgorithm::BreadthFirstSearch,
        grid,
        start,
        destination);
    REQUIRE(direct.explorationOrder == dispatched.explorationOrder);
    REQUIRE(direct.path == dispatched.path);
}

struct TestCase {
    std::string_view name;
    std::function<void()> run;
};

}  // namespace

int main() {
    const std::vector<TestCase> tests = {
        {"grid operations and boundaries", testGridOperationsAndBoundaries},
        {"empty grid", testEmptyGrid},
        {"obstacle grid requiring detour", testObstacleGridAndDetour},
        {"long wall detour", testLongWallDetour},
        {"unreachable destination", testUnreachableDestination},
        {"start equals destination", testStartEqualsDestination},
        {"invalid and blocked endpoints", testInvalidAndBlockedEndpoints},
        {"boundary route", testBoundaryRoute},
        {"repeatability and dispatcher", testRepeatabilityAndDispatcher},
    };

    std::size_t passed = 0;
    for (const TestCase& test : tests) {
        try {
            test.run();
            ++passed;
            std::cout << "[PASS] " << test.name << '\n';
        } catch (const std::exception& error) {
            std::cerr << "[FAIL] " << test.name << "\n  " << error.what() << '\n';
        }
    }

    std::cout << passed << '/' << tests.size() << " tests passed\n";
    return passed == tests.size() ? 0 : 1;
}
