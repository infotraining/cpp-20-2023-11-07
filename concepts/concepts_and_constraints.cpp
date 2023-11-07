#include <cassert>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std::literals;

namespace Traits
{
    template <typename T>
    struct is_void
    {
        static constexpr bool value = false;
    };

    template <>
    struct is_void<void>
    {
        static constexpr bool value = true;
    };

    // template variable
    template <typename T>
    constexpr static bool is_void_v = is_void<T>::value;

    ///////////////////////////////////////////////////////////

    template <typename T>
    struct is_pointer
    {
        static constexpr bool value = false;
    };

    template <typename T>
    struct is_pointer<T*>
    {
        static constexpr bool value = true;
    };

    // template variable
    template <typename T>
    constexpr static bool is_pointer_v = is_pointer<T>::value;

} // namespace Traits

TEST_CASE("type traits")
{
    using namespace Traits;

    SECTION("is_void")
    {
        using SomeType = void;
        using OtherType = int;

        static_assert(is_void_v<SomeType>);
        static_assert(std::is_void_v<OtherType> == false);
    }

    SECTION("is_pointer")
    {
        using SomeType = int*;
        using OtherType = double;

        static_assert(is_pointer_v<SomeType>);
        static_assert(std::is_pointer_v<OtherType> == false);
    }
}

namespace BeforeCpp20
{
    template <typename T>
    void use(T ptr)
    {
        static_assert(Traits::is_pointer_v<T>, "T must be a pointer type");

        if (ptr)
            std::cout << *ptr << "\n";
    }
} // namespace BeforeCpp20

namespace Cpp20
{
    void use(auto ptr)
        requires Traits::is_pointer_v<decltype(ptr)>
    {
        if (ptr)
            std::cout << *ptr << "\n";
    }
} // namespace Cpp20

TEST_CASE("use traits in static assert")
{
    // BeforeCpp20::use("text"s);
    // Cpp20::use("text"s);
}

/////////////////////////////////////////////////////////////////////////

namespace ver_1
{
    template <typename T>
    auto max_value(T a, T b)
    {
        return a < b ? b : a;
    }

    template <typename T>
        requires Traits::is_pointer_v<T>
    auto max_value(T a, T b)
    {
        assert(a != nullptr);
        assert(b != nullptr);
        return *a < *b ? *b : *a;
    }
} // namespace ver_1

namespace ver_2
{
    template <typename T>
    concept Pointer = Traits::is_pointer_v<T>;

    static_assert(Pointer<int*>);
    static_assert(Pointer<const int*>);
    static_assert(!Pointer<const int&>);

    template <typename T>
    auto max_value(T a, T b)
    {
        return a < b ? b : a;
    }

    template <typename T>
        requires Pointer<T> // requires concept
    auto max_value(T a, T b)
    {
        assert(a != nullptr);
        assert(b != nullptr);
        return *a < *b ? *b : *a;
    }

} // namespace ver_2

template <typename T>
concept Pointer = requires(T ptr) {
    *ptr;
    ptr == nullptr;
};

static_assert(Pointer<int*>);
static_assert(Pointer<std::shared_ptr<int>>);
static_assert(Pointer<std::unique_ptr<int>>);

template <typename T>
auto max_value(T a, T b)
{
    return a < b ? b : a;
}

template <Pointer T>
auto max_value(T a, T b)
{
    assert(a != nullptr);
    assert(b != nullptr);
    return max_value(*a, *b);
}

void use(Pointer auto ptr)
{
    if (ptr)
        std::cout << *ptr << "\n";
}

std::integral auto get_id()
{
    return 42.0;
}

TEST_CASE("max_value")
{
    int x = 10;
    int y = 20;
    CHECK(max_value(x, y) == 20);

    CHECK(max_value(&x, &y) == 20);

    Pointer auto ptr1 = std::make_shared<int>(10);
    Pointer auto ptr2 = std::make_shared<int>(20);
    CHECK(max_value(ptr1, ptr2) == 20);

    std::integral auto id = get_id();
}