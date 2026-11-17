#ifndef PATHFINDING_VISUALIZER_PATHFINDING_H
#define PATHFINDING_VISUALIZER_PATHFINDING_H

#include "grid.h"

#include <cstddef>
#include <vector>

enum class PathfindingAlgorithm {
    BreadthFirstSearch,
    DepthFirstSearch,
    Dijkstra,
    AStar,
};

struct PathfindingResult {
    // True only when a path from start to destination was found.
    bool found = false;

    // Nodes in the unique, deterministic order in which the algorithm expanded them.
    std::vector<Position> explorationOrder;

    // Start-to-destination path, including both endpoints; empty when not found.
    std::vector<Position> path;

    // Number of expanded nodes. This always equals explorationOrder.size().
    std::size_t exploredNodes = 0;

    // Number of movement edges in path. Zero for no path or start == destination.
    std::size_t pathLength = 0;

    // C++ search time in milliseconds, excluding frontend rendering and animation.
    double executionTimeMilliseconds = 0.0;
};

PathfindingResult breadthFirstSearch(const Grid& grid, Position start, Position destination);
PathfindingResult depthFirstSearch(const Grid& grid, Position start, Position destination);
PathfindingResult dijkstraSearch(const Grid& grid, Position start, Position destination);
PathfindingResult aStarSearch(const Grid& grid, Position start, Position destination);

PathfindingResult findPath(
    PathfindingAlgorithm algorithm,
    const Grid& grid,
    Position start,
    Position destination);

#endif

