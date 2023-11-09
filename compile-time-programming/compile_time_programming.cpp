#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <helpers.hpp>
#include <array>

using namespace std::literals;

int runtime_func(int x)
{
    return x * x;
}
 
constexpr int constexpr_func(int x) 
{
    return x * x;
}
 
consteval int consteval_func(int x)
{
    return x * x;
}

void compile_time_error() // runtime function
{ }

consteval int next_two_digit_value(int value)
{
    if (value < 9 || value >= 99)
    {
        compile_time_error();
    }

    return ++value;
}

TEST_CASE("runtime vs. constexpr vs. consteval")
{
    int x = 42;
    int r1 = runtime_func(x);   // runtime

    std::array<int, constexpr_func(4)> arr1 = {}; // compile-time
    
    constexpr int cx = 10;
    std::array<int, constexpr_func(cx)> arr2 = {}; // compile-time

    int r2 = constexpr_func(x); // runtime

    constexpr auto r3 = consteval_func(24); // compile-time

    int runtime_variable1 = consteval_func(33); // compile-time
    int runtime_variable2 = consteval_func(cx); // compile-time
    //int runtime_variable = consteval_func(x);  // ERROR

    CHECK(next_two_digit_value(50) == 51);
    //CHECK(next_two_digit_value(99) == 51);
}

TEST_CASE("consteval lambda")
{
    auto square = [](int n) consteval { return n * n; };

    std::array arr = { square(1), square(2), square(3) };
}

constexpr int len(const char* s)
{
    if (std::is_constant_evaluated())
    {
        // compile-time friendly code
        int idx = 0;
        while (s[idx] != '\0')
            ++idx;
        return idx;
    }
    else
    {
       return std::strlen(s); // function called at runtime
    }
}

TEST_CASE("is_constant_evaluated")
{
    int size1 = len("abc");

    constexpr int size2 = len("abc");
}

consteval int get_id()
{
    return 665;
}

constinit int id = get_id();

template <size_t N>
constexpr auto create_powers()
{
    std::array<uint32_t, N> powers{};

    std::iota(powers.begin(), powers.end(), 1); // std algorithms are constexpr

    std::ranges::transform(powers, powers.begin(), [](int x) { return x * x; }); // ranges algorithms are constexpr

    return powers;
}

template <std::ranges::input_range... TRng_>
constexpr auto avg_for_unique(const TRng_&... rng)
{
    using TElement = std::common_type_t<std::ranges::range_value_t<TRng_>...>;

    std::vector<TElement> vec;                            // empty vector
    vec.reserve((rng.size() + ...));                      // reserve a buffer - fold expression C++17
    (vec.insert(vec.end(), rng.begin(), rng.end()), ...); // fold expression C++17

    // sort items
    std::ranges::sort(vec); // std::sort(vec.begin(), vec.end());

    // create span of unique_items
    auto new_end = std::unique(vec.begin(), vec.end());
    std::span unique_items{vec.begin(), new_end};

    // calculate sum of unique items
    auto sum = std::accumulate(unique_items.begin(), unique_items.end(), TElement{});

    return sum / static_cast<double>(unique_items.size());
}

TEST_CASE("avg for unique")
{
    constexpr std::array lst1 = {1, 2, 3, 4, 5};
    constexpr std::array lst2 = {5, 6, 7, 8, 9};

    constexpr auto avg = avg_for_unique(lst1, lst2);

    std::cout << "AVG: " << avg << "\n";
}

TEST_CASE("constexpr extensions")
{
    constexpr auto powers_lookup_table = create_powers<20>();

    CHECK(powers_lookup_table[6] == 36);

}