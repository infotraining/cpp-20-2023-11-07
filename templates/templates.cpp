#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <memory>
#include <numeric>
#include <set>
#include <string>
#include <vector>
#include <array>

using namespace std::literals;

auto fun_cpp20(auto a, auto b)
{
    return a + b;
}

auto lambda_cpp14 = [](auto a, auto b) {
    return a + b;
};

namespace CompilerInterpretation
{
    template <typename T1, typename T2>
    auto fun_cpp20(T1 a, T2 b)
    {
        return a + b;
    }

    class Lambda_472365827364827364
    {
        Lambda_472365827364827364() = delete;

        template <typename T1, typename T2>
        auto operator()(T1 a, T2 b) const
        {
            return a + b;
        }
    };
} // namespace CompilerInterpretation

TEST_CASE("templates & lambda expressions")
{
    auto sum = []<typename T>(const std::vector<T>& v, auto init_value) {
        return std::accumulate(v.begin(), v.end(), init_value);
    };

    std::vector<int> vec = {1, 2, 3};
    CHECK(sum(vec, 0.0) == 6.0);

    auto forward_to_vector = []<typename T, typename... TArgs>(std::vector<T>& v, TArgs&&... args) {
        v.emplace_back(std::forward<TArgs>(args)...);
    };

    forward_to_vector(vec, 4);
}

TEST_CASE("default construction for lambda")
{
    SECTION("before C++20")
    {
        auto cmp_by_value = [](auto a, auto b) {
            return *a < *b;
        };

        std::set<std::shared_ptr<int>, decltype(cmp_by_value)> my_set(cmp_by_value);

        my_set.insert(std::make_shared<int>(42));
        my_set.insert(std::make_shared<int>(1));
        my_set.insert(std::make_shared<int>(665));
        my_set.insert(std::make_shared<int>(65));

        for (const auto& ptr : my_set)
        {
            std::cout << *ptr << " ";
        }
        std::cout << "\n";
    }

    SECTION("since C++20")
    {
        auto cmp_by_value = [](auto a, auto b) {
            return *a < *b;
        };

        decltype(cmp_by_value) another_instance_of_comparer;

        CHECK(cmp_by_value(std::make_shared<int>(41), std::make_shared<int>(42)));
        CHECK(another_instance_of_comparer(std::make_shared<int>(41), std::make_shared<int>(42)));

        std::set<std::shared_ptr<int>, decltype(cmp_by_value)> my_set;

        my_set.insert(std::make_shared<int>(42));
        my_set.insert(std::make_shared<int>(1));
        my_set.insert(std::make_shared<int>(665));
        my_set.insert(std::make_shared<int>(65));

        for (const auto& ptr : my_set)
        {
            std::cout << *ptr << " ";
        }
        std::cout << "\n";
    }
}

auto create_caller(auto f, auto... args)
{
    return [f, ... args = std::move(args)]() -> decltype(auto) {
        return f(args...);
    };
};

TEST_CASE("lambda - capture parameter pack")
{
    auto calulate = create_caller(std::plus{}, 4, 6);
    CHECK(calulate() == 10);
}

//////////////////////////////////////////////////

struct Person
{
    int id;
    std::string name;
    double salary;
    double height;
};

template <typename T1, typename T2>
struct MyPair
{
    T1 fst;
    T2 snd;
};

// // deduction guide
// template<typename T1, typename T2>
// MyPair(T1, T2) -> MyPair<T1, T2>;

TEST_CASE("aggregates")
{
    Person p1{42, "Kowalski"};
    Person p2{.name = "Nowak", .height = 1.78};
    //Person p3{.height = "2.01", .id = 42}; // ERROR

    auto ptr_p = std::make_unique<Person>(42, "Nowak", 10'000.0); // OK
    Person p3(665, "Anonim");
    // Person p4(.id=665, .height=1.88); // ERROR

    int arr1[] = {1, 2, 3, 4, 5};
    int arr2[](1, 2, 3, 4, 5);

    std::array arr3 = {1, 2, 3, 4, 5};
    std::array<int, 5> arr4({1, 2, 3, 4, 5});

    MyPair mp1{20, 3.14};   // MyPair<int, double>
    MyPair mp2("test", 43); // MyPair<const char*, int>
}