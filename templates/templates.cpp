#include <array>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <memory>
#include <numeric>
#include <set>
#include <string>
#include <vector>

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
    // Person p3{.height = "2.01", .id = 42}; // ERROR

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

////////////////////////////////////////////////////////////////////////
// NTTP - Non-Type Template Parameter

template <size_t N>
concept PowerOf2 = std::has_single_bit(N);

template <typename T, size_t N>
    requires PowerOf2<N>
void zero(std::array<T, N>& arr)
{
    for (auto& item : arr)
        item = T{};
}

TEST_CASE("NTTP + concepts")
{
    std::array<int, 1024> arr1;
    zero(arr1);

    std::array<int, 100> arr2;
    // zero(arr2); // ERROR
}

template <double Factor, typename T>
auto scale(T x)
{
    return x * Factor;
}

namespace AlternativeTake
{
    template <auto Factor, typename T>
    auto scale(T x)
    {
        return x * Factor;
    }
} // namespace AlternativeTake

TEST_CASE("NTTP + double, float")
{
    CHECK(scale<42.1>(2) == 84.2);
    CHECK(AlternativeTake::scale<42>(2) == 84);
    CHECK(AlternativeTake::scale<42.1f>(2) == 84.2f);
}

//////////////////////////////////////////////////

struct Tax
{
    double value;

    constexpr Tax(double v)
        : value{v}
    {
        assert(value >= 0 && v < 1);
    }
};

template <Tax Vat> // struct as NTTP
double calc_gross_price(double net_price)
{
    return net_price + net_price * Vat.value;
}

template <int A, int B, int C, typename T>
auto poly_calculate(T x)
{
    return A * x * x + B * x + C;
}

template <typename T>
struct Factors
{
    T A, B, C;
};

namespace SinceCpp20
{
    template <auto f, typename T>
    auto poly_calculate(T x)
    {
        return f.A * x * x + f.B * x + f.C;
    }
} // namespace SinceCpp20

TEST_CASE("NTTP + struct")
{
    constexpr Tax vat_pl{0.23};
    constexpr Tax vat_ger{0.19};

    CHECK(calc_gross_price<vat_pl>(100.0) == 123.0);
    CHECK(calc_gross_price<vat_ger>(100.0) == 119.0);

    constexpr Factors linear_f{0.0, 1.0, 1.0};

    CHECK(SinceCpp20::poly_calculate<linear_f>(1.0) == 2.0);
}

template <size_t N>
struct Str
{
    char value[N];

    constexpr Str(const char (&str)[N])
    {
        std::copy(str, str + N, value);
    }

    friend std::ostream& operator<<(std::ostream& out, const Str& str)
    {
        out << str.value;

        return out;
    }

    auto operator<=>(const Str& other) const = default; // implicitly constexpr
};

template <Str LogPrefix>
struct Logger
{
    void log(std::string_view msg)
    {
        std::cout << LogPrefix << msg << "\n";
    }
};

TEST_CASE("NTTP + strings")
{
    constexpr Str txt1{"text"};
    constexpr Str txt2{"text"};

    static_assert(txt1 == txt2);

    Logger<">: "> my_logger_1;
    my_logger_1.log("Start");

    Logger<"log: "> my_logger_2;
    my_logger_2.log("End");
}

template <std::invocable auto GetVat>
double calc_gross_price(double net_price)
{
    return net_price + net_price * GetVat();
}

TEST_CASE("NTTP + lambda")
{
    CHECK(calc_gross_price<[]{ return 0.23; }>(100.0) == 123.0);

    constexpr static auto vat_ger = []{ return 0.19; };
    CHECK(calc_gross_price<vat_ger>(100.0) == 119.0);
}