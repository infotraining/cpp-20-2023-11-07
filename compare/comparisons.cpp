#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace std::literals;

TEST_CASE("safe comparisons between integral numbers")
{
    int x = -7;
    unsigned y = 42;

    // CHECK(x < y);
    CHECK(std::cmp_less(x, y));
    CHECK(std::cmp_greater_equal(y, x));

    CHECK(std::in_range<size_t>(-1) == false);
    CHECK(std::in_range<size_t>(665) == true);

    // auto my_cmp_less = [](std::integral auto a, std::integral auto b) { return std::cmp_less(a, b); };
    auto my_cmp_less = [](auto a, auto b) {
        if constexpr (std::is_integral_v<decltype(a)> && std::is_integral_v<decltype(b)>)
            return std::cmp_less(a, b);
        else
            return a < b;
    };

    CHECK(my_cmp_less(x, y));

    std::vector vec = {1, 2, 3};
    CHECK(my_cmp_less(x, vec.size()));

    CHECK(my_cmp_less("test"s, "text"s));
}

/////////////////////////////////////////////////////////////////////////////////

struct Point
{
    int x, y;

    // bool operator==(const Point& other) const
    // {
    //     // return x == other.x && y == other.y;
    //     return std::tie(x, y) == std::tie(other.x, other.y);
    // }

    bool operator==(const Point& other) const = default; // since C++20

    bool operator==(const std::pair<int, int>& other) const
    {
        return x == other.first && y == other.second;
    }
};

struct NamedPoint : Point
{
    std::string name = "not-set";

    bool operator==(const NamedPoint& other) const = default;
};

TEST_CASE("operator ==")
{
    SECTION("expression rewriting")
    {
        Point pt{10, 20};

        CHECK(pt == Point{10, 20});
        CHECK(pt != Point{0, 10}); // !(pt == Point{0, 10})

        CHECK(pt == std::pair{10, 20}); // pt.operator==(std::pair{10, 20});
        CHECK(std::pair{10, 20} == pt); // pt == std::pair{10, 20}
        CHECK(std::pair{10, 21} != pt); // pt == std::pair{10, 20}
    }

    SECTION("default operator==")
    {
        NamedPoint origin{{0, 0}, "origin"};

        CHECK(origin == NamedPoint{{0, 0}, "origin"});
        CHECK(origin != NamedPoint{{0, 0}, "other"});
    }
}

struct Number
{
    int value;

    Number(int v)
        : value{v}
    { }

    // bool operator==(const Number&) const = default;

    // auto operator<=>(const Number& other) const
    // {
    //     return value <=> other.value;
    // }

    auto operator<=>(const Number& other) const = default;
};

TEST_CASE("order")
{
    std::vector<Number> numbers = {Number{10}, {20}, {-5}, {8}, {665}};

    std::ranges::sort(numbers);
    CHECK(std::ranges::is_sorted(numbers));

    Number n{42};
    CHECK(n == Number{42});
    CHECK(42 == n);
    CHECK(n != Number{665});
    CHECK(n < Number{665});
    CHECK(n > Number{6});
    CHECK(n >= Number{6});
    CHECK(n <= Number{42});

    auto result = n <=> Number{10};
}

TEST_CASE("three-way comparison operator")
{
    SECTION("compare to 0")
    {
        bool result = 1 <=> 5 < 0;
        CHECK(result == true);

        result = 5 <=> 1 > 0;
        CHECK(result == true);

        result = 5 <=> 5 == 0;
        CHECK(result == true);
    }

    SECTION("comparison categories")
    {
        SECTION("strong_ordering")
        {
            CHECK(4 <=> 4 == std::strong_ordering::equal);
            CHECK(4 <=> 5 == std::strong_ordering::less);
            CHECK(5 <=> 4 == std::strong_ordering::greater);

            auto result = 6 <=> 8; // std::strong_ordering
        }

        SECTION("partial_ordering")
        {
            CHECK(3.14 <=> 4.13 == std::partial_ordering::less);
            CHECK(3.14 <=> 3.14 == std::partial_ordering::equivalent);
            CHECK(4.34 <=> std::numeric_limits<double>::quiet_NaN() == std::partial_ordering::unordered);
        }
    }
}


struct FloatNumber
{
    float value;

    FloatNumber(float v) : value{v}
    {}

    auto operator<=>(const FloatNumber& other) const = default;
};

TEST_CASE("FloatNumber <=>")
{
    CHECK(FloatNumber{41.1f} <=> FloatNumber{77.7f} == std::partial_ordering::less);

    CHECK(FloatNumber{0.0f} < FloatNumber{6.66f}); // (FloatNumber{0.0f} <=> FloatNumber{6.66f}) < 0
}


struct Human
{
    std::string name; // std::strong_ordering
    uint8_t age;      // std::strong_ordering 
    double height;    // std::partial_ordering

    auto operator<=>(const Human& other) const = default;
};

TEST_CASE("many fields in class/struct")
{
    Human jan1{"Jan", 42, 1.77};
    Human jan2{"Jan", 42, 1.87};

    CHECK(jan1 < jan2);
}