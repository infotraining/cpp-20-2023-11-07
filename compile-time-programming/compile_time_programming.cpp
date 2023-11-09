#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <helpers.hpp>

using namespace std::literals;

TEST_CASE("constinit")
{
    REQUIRE(true);
}

TEST_CASE("consteval")
{
    REQUIRE(true);
}

TEST_CASE("constexpr extensions")
{
    constexpr auto data = helpers::create_numeric_dataset<20>();
}