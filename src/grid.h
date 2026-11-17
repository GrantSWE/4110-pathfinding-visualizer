#ifndef PATHFINDING_VISUALIZER_GRID_H
#define PATHFINDING_VISUALIZER_GRID_H

#include <array>
#include <cstddef>
#include <vector>

struct Position {
    int row;
    int column;

    bool operator==(const Position& other) const noexcept;
    bool operator!=(const Position& other) const noexcept;
};

class Grid {
public:
    static constexpr int kRows = 20;
    static constexpr int kColumns = 20;
    static constexpr std::size_t kCellCount = kRows * kColumns;

    Grid();

    bool isInBounds(Position position) const noexcept;
    bool isBlocked(Position position) const noexcept;
    bool isTraversable(Position position) const noexcept;

    bool setObstacle(Position position, bool blocked = true) noexcept;
    bool removeObstacle(Position position) noexcept;
    void clearObstacles() noexcept;

    std::size_t toIndex(Position position) const;
    Position toPosition(std::size_t index) const;

    bool hasValidEndpoints(Position start, Position destination) const noexcept;

    // Returns traversable neighbors in deterministic order: up, right, down, left.
    std::vector<Position> validNeighbors(Position position) const;

private:
    std::array<bool, kCellCount> blocked_;
};

#endif

