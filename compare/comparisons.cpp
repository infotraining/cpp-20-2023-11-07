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

    FloatNumber(float v)
        : value{v}
    { }

    auto operator<=>(const FloatNumber& other) const = default;
};

namespace SaferFloatComparison
{
    struct FloatNumber
    {
        float value;

        FloatNumber(float v)
            : value{v}
        { }

        bool operator==(const FloatNumber&) const = default;

        auto operator<=>(const FloatNumber& other) const
        {
            return std::strong_order(value, other.value);
        }
    };
} // namespace SaferFloatComparison

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

    // auto operator<=>(const Human& other) const = default;

    auto tied() const
    {
        return std::tie(name, age);
    }

    bool operator==(const Human& other) const
    {
        return tied() == other.tied();
    }

    std::strong_ordering operator<=>(const Human& other) const
    {
        // if (auto cmp_result = name <=> other.name; cmp_result == 0)
        // {
        //     return age <=> other.age;
        // }
        // else
        // {
        //     return cmp_result;
        // }

        return tied() <=> other.tied();
    }
};

TEST_CASE("many fields in class/struct")
{
    Human jan1{"Jan", 42, 1.77};
    Human jan2{"Jan", 42, 1.87};

    CHECK(jan1 == jan2);

    // CHECK(jan1 < jan2);
    //CHECK(jan1 <=> jan2 == 0);
}

struct CIString
{
    std::string str;

    std::string to_upper_copy() const
    {
        std::string upper_str{str};
        std::ranges::transform(upper_str, upper_str.begin(), [](auto c) { return std::toupper(c); });
        return upper_str;
    }

    bool operator==(const CIString& other) const
    {
        std::string upper_str_left = to_upper_copy();
        std::string upper_str_right = other.to_upper_copy();

        return upper_str_left == upper_str_right;
    }

    std::weak_ordering operator<=>(const CIString& other) const
    {
        std::string upper_str_left = to_upper_copy();
        std::string upper_str_right = other.to_upper_copy();

        // if (upper_str_left == upper_str_right)
        //     return std::weak_ordering::equivalent;
        // if (upper_str_left < upper_str_right)
        //     return std::weak_ordering::less;
        // return std::weak_ordering::greater;

        return upper_str_left <=> upper_str_right;
    }
};

TEST_CASE("Case-insensitive string")
{
    auto result = CIString{"ala"} <=> CIString{"Ala"};

    CHECK(result == std::weak_ordering::equivalent);
    CHECK(CIString{"ala"} == CIString{"Ala"});
}

struct Base
{
    std::string value;

    bool operator==(const Base& other) const { return value == other.value; }
    bool operator<(const Base& other) const { return value < other.value; }
};

struct Derived : Base
{
    std::vector<int> data;
 
    std::strong_ordering operator<=>(const Derived& other) const = default;
};

TEST_CASE("default <=> - how it works")
{
    Derived d1{{"text"}, {1, 2, 3}};
    Derived d2{{"text"}, {1, 2, 4}};

    CHECK(d1 < d2);
}

struct Data
{
    int* buffer_;
    std::size_t size_;
public:
    Data(std::initializer_list<int> lst) : buffer_{new int[lst.size()]}, size_{lst.size()}
    {
        std::ranges::copy(lst, buffer_);
    }

    Data(const Data&) = delete;
    Data& operator=(const Data&) = delete;

    bool operator==(const Data& other) const 
    {
        return size_ == other.size_ && std::equal(buffer_, buffer_ + size_, other.buffer_);
    }

    auto operator<=>(const Data& other) const
    {
        return std::lexicographical_compare_three_way(buffer_, buffer_ + size_, other.buffer_, other.buffer_ + other.size_);
    }

    ~Data()
    {
        delete[] buffer_;
    }
};

TEST_CASE("Data - comparisons")
{
    Data ds1{1, 2, 3};
    Data ds2{1, 2, 3};
    Data ds3{1, 2, 4};

    CHECK(ds1 == ds2);
    CHECK(ds1 != ds3);

    CHECK(ds1 < ds3);
}