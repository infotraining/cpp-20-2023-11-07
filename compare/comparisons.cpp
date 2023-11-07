#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <utility>

using namespace std::literals;

TEST_CASE("safe comparisons between integral numbers")
{
    int x = -7;
    unsigned y = 42;

    //CHECK(x < y);
    CHECK(std::cmp_less(x, y));
    CHECK(std::cmp_greater_equal(y, x));

    CHECK(std::in_range<size_t>(-1) == false);
    CHECK(std::in_range<size_t>(665) == true);

    //auto my_cmp_less = [](std::integral auto a, std::integral auto b) { return std::cmp_less(a, b); };
    auto my_cmp_less = [](auto a, auto b) { 
        if constexpr(std::is_integral_v<decltype(a)> && std::is_integral_v<decltype(b)>)
            return std::cmp_less(a, b);
        else
            return a < b; 
    };

    CHECK(my_cmp_less(x, y));

    std::vector vec = {1, 2, 3};
    CHECK(my_cmp_less(x, vec.size()));

    CHECK(my_cmp_less("test"s, "text"s));
}
