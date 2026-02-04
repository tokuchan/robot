cmake_minimum_required(VERSION 3.24)
project(robot CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Boost REQUIRED)
find_package(Catch2 REQUIRED)

# Main library or executable
add_library(robot_lib src/robot.cpp)
target_link_libraries(robot_lib PUBLIC Boost::boost)
target_include_directories(robot_lib PUBLIC include)

# Enable testing
enable_testing()

# Unit tests with Catch2
add_executable(robot_tests tests/catch_tests.cpp)
target_link_libraries(robot_tests PRIVATE robot_lib Catch2::Catch2)
target_compile_definitions(robot_tests PRIVATE CATCH_CONFIG_MAIN)
add_test(NAME Catch2Tests COMMAND robot_tests)