#include "pathfinding.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <limits>
#include <queue>
#include <stack>
#include <tuple>
#include <utility>

namespace {

using Clock = std::chrono::steady_clock;
constexpr std::size_t kNoParent = Grid::kCellCount;
constexpr int kInfinity = std::numeric_limits<int>::max();

std::vector<Position> reconstructPath(
    const Grid& grid,
    const std::array<std::size_t, Grid::kCellCount>& parent,
    Position start,
    Position destination) {
    const std::size_t startIndex = grid.toIndex(start);
    std::size_t currentIndex = grid.toIndex(destination);
    std::vector<Position> reversedPath;

    while (true) {
        reversedPath.push_back(grid.toPosition(currentIndex));
        if (currentIndex == startIndex) {
            break;
        }
        if (parent[currentIndex] == kNoParent) {
            return {};
        }
        currentIndex = parent[currentIndex];
    }

    std::reverse(reversedPath.begin(), reversedPath.end());
    return reversedPath;
}

void finishResult(PathfindingResult& result, Clock::time_point startTime) {
    result.exploredNodes = result.explorationOrder.size();
    result.pathLength = result.found && !result.path.empty() ? result.path.size() - 1 : 0;
    result.executionTimeMilliseconds =
        std::chrono::duration<double, std::milli>(Clock::now() - startTime).count();
}

int manhattanDistance(Position first, Position second) {
    return std::abs(first.row - second.row) + std::abs(first.column - second.column);
}

struct DijkstraEntry {
    int distance;
    std::size_t index;
};

struct DijkstraEntryGreater {
    bool operator()(const DijkstraEntry& left, const DijkstraEntry& right) const noexcept {
        return std::tie(left.distance, left.index) > std::tie(right.distance, right.index);
    }
};

struct AStarEntry {
    int estimatedTotalCost;
    int heuristic;
    int distanceFromStart;
    std::size_t index;
};

struct AStarEntryGreater {
    bool operator()(const AStarEntry& left, const AStarEntry& right) const noexcept {
        return std::tie(
                   left.estimatedTotalCost,
                   left.heuristic,
                   left.distanceFromStart,
                   left.index) >
               std::tie(
                   right.estimatedTotalCost,
                   right.heuristic,
                   right.distanceFromStart,
                   right.index);
    }
};

}  // namespace

PathfindingResult breadthFirstSearch(const Grid& grid, Position start, Position destination) {
    const auto startTime = Clock::now();
    PathfindingResult result;

    if (!grid.hasValidEndpoints(start, destination)) {
        finishResult(result, startTime);
        return result;
    }

    std::array<bool, Grid::kCellCount> discovered{};
    std::array<std::size_t, Grid::kCellCount> parent;
    parent.fill(kNoParent);

    std::queue<Position> frontier;
    frontier.push(start);
    discovered[grid.toIndex(start)] = true;

    while (!frontier.empty()) {
        const Position current = frontier.front();
        frontier.pop();
        result.explorationOrder.push_back(current);

        if (current == destination) {
            result.found = true;
            break;
        }

        const std::size_t currentIndex = grid.toIndex(current);
        for (const Position neighbor : grid.validNeighbors(current)) {
            const std::size_t neighborIndex = grid.toIndex(neighbor);
            if (!discovered[neighborIndex]) {
                discovered[neighborIndex] = true;
                parent[neighborIndex] = currentIndex;
                frontier.push(neighbor);
            }
        }
    }

    if (result.found) {
        result.path = reconstructPath(grid, parent, start, destination);
    }
    finishResult(result, startTime);
    return result;
}

PathfindingResult depthFirstSearch(const Grid& grid, Position start, Position destination) {
    const auto startTime = Clock::now();
    PathfindingResult result;

    if (!grid.hasValidEndpoints(start, destination)) {
        finishResult(result, startTime);
        return result;
    }

    std::array<bool, Grid::kCellCount> discovered{};
    std::array<std::size_t, Grid::kCellCount> parent;
    parent.fill(kNoParent);

    std::stack<Position> frontier;
    frontier.push(start);
    discovered[grid.toIndex(start)] = true;

    while (!frontier.empty()) {
        const Position current = frontier.top();
        frontier.pop();
        result.explorationOrder.push_back(current);

        if (current == destination) {
            result.found = true;
            break;
        }

        const std::size_t currentIndex = grid.toIndex(current);
        const std::vector<Position> neighbors = grid.validNeighbors(current);
        for (auto iterator = neighbors.rbegin(); iterator != neighbors.rend(); ++iterator) {
            const std::size_t neighborIndex = grid.toIndex(*iterator);
            if (!discovered[neighborIndex]) {
                discovered[neighborIndex] = true;
                parent[neighborIndex] = currentIndex;
                frontier.push(*iterator);
            }
        }
    }

    if (result.found) {
        result.path = reconstructPath(grid, parent, start, destination);
    }
    finishResult(result, startTime);
    return result;
}

PathfindingResult dijkstraSearch(const Grid& grid, Position start, Position destination) {
    const auto startTime = Clock::now();
    PathfindingResult result;

    if (!grid.hasValidEndpoints(start, destination)) {
        finishResult(result, startTime);
        return result;
    }

    std::array<int, Grid::kCellCount> distance;
    distance.fill(kInfinity);
    std::array<std::size_t, Grid::kCellCount> parent;
    parent.fill(kNoParent);
    std::array<bool, Grid::kCellCount> expanded{};

    const std::size_t startIndex = grid.toIndex(start);
    distance[startIndex] = 0;

    std::priority_queue<DijkstraEntry, std::vector<DijkstraEntry>, DijkstraEntryGreater> frontier;
    frontier.push({0, startIndex});

    while (!frontier.empty()) {
        const DijkstraEntry entry = frontier.top();
        frontier.pop();

        if (expanded[entry.index] || entry.distance != distance[entry.index]) {
            continue;
        }

        expanded[entry.index] = true;
        const Position current = grid.toPosition(entry.index);
        result.explorationOrder.push_back(current);

        if (current == destination) {
            result.found = true;
            break;
        }

        for (const Position neighbor : grid.validNeighbors(current)) {
            const std::size_t neighborIndex = grid.toIndex(neighbor);
            const int candidateDistance = entry.distance + 1;
            if (candidateDistance < distance[neighborIndex]) {
                distance[neighborIndex] = candidateDistance;
                parent[neighborIndex] = entry.index;
                frontier.push({candidateDistance, neighborIndex});
            }
        }
    }

    if (result.found) {
        result.path = reconstructPath(grid, parent, start, destination);
    }
    finishResult(result, startTime);
    return result;
}

PathfindingResult aStarSearch(const Grid& grid, Position start, Position destination) {
    const auto startTime = Clock::now();
    PathfindingResult result;

    if (!grid.hasValidEndpoints(start, destination)) {
        finishResult(result, startTime);
        return result;
    }

    std::array<int, Grid::kCellCount> distance;
    distance.fill(kInfinity);
    std::array<std::size_t, Grid::kCellCount> parent;
    parent.fill(kNoParent);
    std::array<bool, Grid::kCellCount> expanded{};

    const std::size_t startIndex = grid.toIndex(start);
    const int startHeuristic = manhattanDistance(start, destination);
    distance[startIndex] = 0;

    std::priority_queue<AStarEntry, std::vector<AStarEntry>, AStarEntryGreater> frontier;
    frontier.push({startHeuristic, startHeuristic, 0, startIndex});

    while (!frontier.empty()) {
        const AStarEntry entry = frontier.top();
        frontier.pop();

        if (expanded[entry.index] || entry.distanceFromStart != distance[entry.index]) {
            continue;
        }

        expanded[entry.index] = true;
        const Position current = grid.toPosition(entry.index);
        result.explorationOrder.push_back(current);

        if (current == destination) {
            result.found = true;
            break;
        }

        for (const Position neighbor : grid.validNeighbors(current)) {
            const std::size_t neighborIndex = grid.toIndex(neighbor);
            const int candidateDistance = entry.distanceFromStart + 1;
            if (candidateDistance < distance[neighborIndex]) {
                const int heuristic = manhattanDistance(neighbor, destination);
                distance[neighborIndex] = candidateDistance;
                parent[neighborIndex] = entry.index;
                frontier.push({candidateDistance + heuristic, heuristic, candidateDistance, neighborIndex});
            }
        }
    }

    if (result.found) {
        result.path = reconstructPath(grid, parent, start, destination);
    }
    finishResult(result, startTime);
    return result;
}

PathfindingResult findPath(
    PathfindingAlgorithm algorithm,
    const Grid& grid,
    Position start,
    Position destination) {
    switch (algorithm) {
        case PathfindingAlgorithm::BreadthFirstSearch:
            return breadthFirstSearch(grid, start, destination);
        case PathfindingAlgorithm::DepthFirstSearch:
            return depthFirstSearch(grid, start, destination);
        case PathfindingAlgorithm::Dijkstra:
            return dijkstraSearch(grid, start, destination);
        case PathfindingAlgorithm::AStar:
            return aStarSearch(grid, start, destination);
    }

    return {};
}

