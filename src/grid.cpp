#include "grid.h"

#include <stdexcept>

bool Position::operator==(const Position& other) const noexcept {
    return row == other.row && column == other.column;
}

bool Position::operator!=(const Position& other) const noexcept {
    return !(*this == other);
}

Grid::Grid() : blocked_{} {}

bool Grid::isInBounds(Position position) const noexcept {
    return position.row >= 0 && position.row < kRows && position.column >= 0 &&
           position.column < kColumns;
}

bool Grid::isBlocked(Position position) const noexcept {
    return isInBounds(position) && blocked_[static_cast<std::size_t>(position.row * kColumns + position.column)];
}

bool Grid::isTraversable(Position position) const noexcept {
    return isInBounds(position) && !isBlocked(position);
}

bool Grid::setObstacle(Position position, bool blocked) noexcept {
    if (!isInBounds(position)) {
        return false;
    }

    blocked_[static_cast<std::size_t>(position.row * kColumns + position.column)] = blocked;
    return true;
}

bool Grid::removeObstacle(Position position) noexcept {
    return setObstacle(position, false);
}

void Grid::clearObstacles() noexcept {
    blocked_.fill(false);
}

std::size_t Grid::toIndex(Position position) const {
    if (!isInBounds(position)) {
        throw std::out_of_range("Grid position is outside the 20x20 grid");
    }

    return static_cast<std::size_t>(position.row * kColumns + position.column);
}

Position Grid::toPosition(std::size_t index) const {
    if (index >= kCellCount) {
        throw std::out_of_range("Grid index is outside the 20x20 grid");
    }

    return {static_cast<int>(index / kColumns), static_cast<int>(index % kColumns)};
}

bool Grid::hasValidEndpoints(Position start, Position destination) const noexcept {
    return isTraversable(start) && isTraversable(destination);
}

std::vector<Position> Grid::validNeighbors(Position position) const {
    if (!isInBounds(position)) {
        return {};
    }

    constexpr std::array<Position, 4> offsets = {
        Position{-1, 0},
        Position{0, 1},
        Position{1, 0},
        Position{0, -1},
    };

    std::vector<Position> neighbors;
    neighbors.reserve(offsets.size());

    for (const Position offset : offsets) {
        const Position neighbor{position.row + offset.row, position.column + offset.column};
        if (isTraversable(neighbor)) {
            neighbors.push_back(neighbor);
        }
    }

    return neighbors;
}

