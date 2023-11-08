#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <span>
#include <string>
#include <vector>
#include <format>
#include <numbers>
#include <numeric>

using namespace std::literals;

void print(std::span<const int> data)
{
    for (const auto& item : data)
        std::cout << item << " ";
    std::cout << "\n";
}

void zero(std::span<int> data, int zero_value = 0)
{
    for (auto& item : data)
    {
        item = zero_value;
    }
}

TEST_CASE("std::span")
{
    std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    print(vec);

    int tab[10] = {1, 2, 3, 4};
    print(tab);

    zero(vec);
    print(vec);

    zero(std::span{tab, 5}, -1);
    print(tab);
}

TEST_CASE("std::span - api")
{
    int native_array[10] = {1, 2, 3, 4, 5};
    std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    SECTION("subspan")
    {

        std::span<int> span_view = native_array;
        auto part_view = span_view.subspan(2, 4);

        print(part_view);
    }

    SECTION("fixed extent")
    {
        std::span<int, 10> span_view(native_array);
        CHECK(span_view.size() == 10);

        span_view = std::span<int, 10>(vec.data(), 10);
    }

    SECTION("dynamic extent")
    {
        std::span<int> span_view(vec);
        print(span_view);

        span_view = std::span(native_array, 5);
        print(span_view); 
    }
}

void print_as_bytes(const float f, const std::span<const std::byte> bytes)
{
    std::cout << std::format("{:+6}", f) << " - { ";
    
    for(std::byte b : bytes)
    {
        std::cout << std::format("{:02X} ", std::to_integer<int>(b));
    }

    std::cout << "}\n";
}

TEST_CASE("span of bytes")
{
    float data[] = { std::numbers::pi_v<float> };

    std::span<const std::byte> const_bytes = std::as_bytes(std::span{data});
    print_as_bytes(data[0], const_bytes);

    std::span<std::byte> writeable_bytes = std::as_writable_bytes(std::span{data});
    writeable_bytes[3] |= std::byte{0b1000'0000};
    print_as_bytes(data[0], const_bytes);
}

TEST_CASE("subspans")
{
    std::vector<int> vec(100);
    std::iota(vec.begin(), vec.end(), 0);

    print(vec);

    size_t col_size = 10;

    std::cout << "\n";

    for(size_t row = 0; row < vec.size() / col_size; ++row)
    {
        auto row_data = std::span{vec}.subspan(row * col_size, col_size);

        print(row_data);
    }
}

std::span<int> get_head(std::span<int> items, size_t head_size = 1)
{
    return items.first(head_size);
}

TEST_CASE("dangling pointers with span")
{
    std::vector vec = {1, 2, 3, 4, 5};
    auto head = get_head(vec, 3);

    print(head);

    vec.push_back(6);
    //print(head); // UB
}