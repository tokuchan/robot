cmake_minimum_required(VERSION 3.24)
project(robot CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Use vcpkg toolchain if available
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

# Use new Boost find behavior (CMP0167)
cmake_policy(SET CMP0167 NEW)
# Use new behavior for <PACKAGENAME>_ROOT variables (CMP0144)
cmake_policy(SET CMP0144 NEW)

# Find required packages using standard find modules
find_package(Boost REQUIRED COMPONENTS json)
find_package(Catch2 3 REQUIRED)

# Main library or executable
add_executable(robot src/robot.cpp)
target_link_libraries(robot PUBLIC Boost::json)
target_include_directories(robot PUBLIC include)

# Enable testing
enable_testing()

# Unit tests with Catch2
file(GLOB TEST_SOURCES "tests/*_tests.cpp")
add_executable(robot_tests ${TEST_SOURCES})
target_link_libraries(robot_tests PRIVATE Catch2::Catch2WithMain)
target_link_libraries(robot_tests PUBLIC Boost::json)
target_include_directories(robot_tests PUBLIC include src)
add_test(NAME Catch2Tests COMMAND robot_tests)