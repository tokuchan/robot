#include <catch2/catch_test_macros.hpp>
#include "../include/robot.hpp"

TEST_CASE("String to upper", "[string]") {
    REQUIRE(to_upper_string("hello") == "HELLO");
}

TEST_CASE("String to lower", "[string]") {
    REQUIRE(to_lower_string("HELLO") == "hello");
}
