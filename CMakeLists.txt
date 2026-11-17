cmake_minimum_required(VERSION 3.16)

project(PathfindingVisualizer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

add_library(pathfinding_engine
    src/grid.cpp
    src/pathfinding.cpp
)

target_include_directories(pathfinding_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(pathfinding_engine PUBLIC cxx_std_17)

add_executable(pathfinding_demo src/main.cpp)
target_link_libraries(pathfinding_demo PRIVATE pathfinding_engine)

if(BUILD_TESTING)
    add_executable(pathfinding_tests tests/test_pathfinding.cpp)
    target_link_libraries(pathfinding_tests PRIVATE pathfinding_engine)

    add_test(NAME pathfinding_tests COMMAND pathfinding_tests)
endif()
