#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <map>

using namespace std::literals;

auto multiply(auto a, auto b)
{
    return a * b;
}

template <typename T>
auto add(auto a, T b)
{
    return a * b;
}

namespace IsInterpretedAs
{
    template <typename T1, typename T2>
    auto multiply(T1 a, T2 b)
    {
        return a * b;
    }
}

TEST_CASE("auto + templates")
{
    CHECK(multiply(2, 4) == 8);
    CHECK(multiply(2, 3.14) == 6.28);

    //multiply("abc"s, "def");
}

void print_container(const auto& container, std::string_view name)
{
    std::cout << name << ": { ";
    for(const auto& item : container)
        std::cout << item << " ";
    std::cout << "}\n";
};

TEST_CASE("print container")
{
    std::vector vec = {1, 2, 3};
    print_container(vec, "vec");
}

/////////////////////////////////////////////

decltype(auto) call_wrapper(auto f, auto&&... args)
{
    std::cout << "Calling a function!\n";

    return f(std::forward<decltype(args)>(args)...);
}

int foo(int x)
{
    return x * 2;
}

TEST_CASE("call wrapper")
{
    auto result = call_wrapper(foo, 42);
    CHECK(result == 84);
}