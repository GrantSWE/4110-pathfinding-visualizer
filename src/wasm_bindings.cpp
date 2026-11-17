#include "grid.h"
#include "pathfinding.h"

#include <emscripten/bind.h>

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace {

struct WasmPathfindingResult {
    bool validInput = false;
    std::string errorMessage;
    bool found = false;
    std::vector<int> explorationOrder;
    std::vector<int> path;
    int pathLength = 0;
    int exploredNodes = 0;
    double executionTimeMilliseconds = 0.0;
};

bool parseAlgorithm(int identifier, PathfindingAlgorithm& algorithm) {
    switch (identifier) {
        case 0:
            algorithm = PathfindingAlgorithm::BreadthFirstSearch;
            return true;
        case 1:
            algorithm = PathfindingAlgorithm::DepthFirstSearch;
            return true;
        case 2:
            algorithm = PathfindingAlgorithm::Dijkstra;
            return true;
        case 3:
            algorithm = PathfindingAlgorithm::AStar;
            return true;
        default:
            return false;
    }
}

bool isValidCellIndex(int index) {
    return index >= 0 && index < static_cast<int>(Grid::kCellCount);
}

std::vector<int> positionsToIndices(const Grid& grid, const std::vector<Position>& positions) {
    std::vector<int> indices;
    indices.reserve(positions.size());
    for (const Position position : positions) {
        indices.push_back(static_cast<int>(grid.toIndex(position)));
    }
    return indices;
}

WasmPathfindingResult invalidResult(std::string message) {
    WasmPathfindingResult result;
    result.errorMessage = std::move(message);
    return result;
}

WasmPathfindingResult runPathfindingForJavaScript(
    int algorithmIdentifier,
    int startIndex,
    int destinationIndex,
    const std::vector<int>& blockedCellIndices) {
    PathfindingAlgorithm algorithm = PathfindingAlgorithm::BreadthFirstSearch;
    if (!parseAlgorithm(algorithmIdentifier, algorithm)) {
        return invalidResult("Algorithm identifier must be 0 (BFS), 1 (DFS), 2 (Dijkstra), or 3 (A*).");
    }
    if (!isValidCellIndex(startIndex)) {
        return invalidResult("Start cell index must be between 0 and 399.");
    }
    if (!isValidCellIndex(destinationIndex)) {
        return invalidResult("Destination cell index must be between 0 and 399.");
    }

    Grid grid;
    for (const int blockedIndex : blockedCellIndices) {
        if (!isValidCellIndex(blockedIndex)) {
            return invalidResult("Every blocked cell index must be between 0 and 399.");
        }
        grid.setObstacle(grid.toPosition(static_cast<std::size_t>(blockedIndex)));
    }

    const Position start = grid.toPosition(static_cast<std::size_t>(startIndex));
    const Position destination = grid.toPosition(static_cast<std::size_t>(destinationIndex));
    if (!grid.hasValidEndpoints(start, destination)) {
        return invalidResult("Start and destination cells must not be blocked.");
    }

    const PathfindingResult nativeResult = findPath(algorithm, grid, start, destination);

    WasmPathfindingResult result;
    result.validInput = true;
    result.found = nativeResult.found;
    result.explorationOrder = positionsToIndices(grid, nativeResult.explorationOrder);
    result.path = positionsToIndices(grid, nativeResult.path);
    result.pathLength = static_cast<int>(nativeResult.pathLength);
    result.exploredNodes = static_cast<int>(nativeResult.exploredNodes);
    result.executionTimeMilliseconds = nativeResult.executionTimeMilliseconds;
    return result;
}

}  // namespace

EMSCRIPTEN_BINDINGS(pathfinding_visualizer_bindings) {
    emscripten::register_vector<int>("IntVector");

    emscripten::value_object<WasmPathfindingResult>("PathfindingResult")
        .field("validInput", &WasmPathfindingResult::validInput)
        .field("errorMessage", &WasmPathfindingResult::errorMessage)
        .field("found", &WasmPathfindingResult::found)
        .field("explorationOrder", &WasmPathfindingResult::explorationOrder)
        .field("path", &WasmPathfindingResult::path)
        .field("pathLength", &WasmPathfindingResult::pathLength)
        .field("exploredNodes", &WasmPathfindingResult::exploredNodes)
        .field("executionTimeMilliseconds", &WasmPathfindingResult::executionTimeMilliseconds);

    emscripten::function("runPathfinding", &runPathfindingForJavaScript);
}
