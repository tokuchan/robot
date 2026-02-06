cmake_minimum_required(VERSION 3.24)
project(robot CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages
find_package(Boost REQUIRED)
find_package(Catch2 3 REQUIRED)

# Main library or executable
add_executable(robot src/robot.cpp)
target_link_libraries(robot PUBLIC Boost::boost)
target_include_directories(robot PUBLIC include)

# Enable testing
enable_testing()

# Unit tests with Catch2
file(GLOB TEST_SOURCES "tests/*_tests.cpp")
add_executable(robot_tests ${TEST_SOURCES})
target_link_libraries(robot_tests PRIVATE Catch2::Catch2WithMain)
target_link_libraries(robot_tests PUBLIC Boost::boost)
target_include_directories(robot_tests PUBLIC include src)
add_test(NAME Catch2Tests COMMAND robot_tests)