#include <cassert>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <numeric>
#include <set>

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
    return 42;
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

/////////////////////////////////////////////////////////////////

namespace Training
{
    template <typename T>
    concept Integral = std::is_integral_v<T>;

    template <typename T>
    concept SignedIntegral = Integral<T> && std::is_signed_v<T>;

    template <typename T>
    concept UnsignedIntegral = Integral<T> && !SignedIntegral<T>;

    template <typename T>
    concept Addable = requires(T a, T b) {
        a + b;
    };

    template <Integral T>
    T add(T a, T b)
    {
        return a + b;
    }

    template <typename T>
    concept Hashable = requires(T a) {
        {
            std::hash<T>{}(a)
        } -> std::convertible_to<std::size_t>;
    };

    template <typename T>
    concept Printable = requires(T& obj, std::ostream& out) { out << obj; };

    template <typename T>
    concept PrintableRange = std::ranges::range<T> && Printable<std::ranges::range_value_t<T>>;

    template <PrintableRange T>
    void print(const T& container, std::string_view name)
    {
        std::cout << name << ": { ";
        for (const auto& item : container)
            std::cout << item << " ";
        std::cout << "}\n";
    };

    template <typename T>
    struct Wrapper
    {
        T value;

        void print() const
            requires Printable<T>
        {
            std::cout << "value: " << value << "\n";
        }

        void print() const
            requires PrintableRange<T>
        {
            Training::print(value, "values");
        }
    };

    template <typename T>
    concept Sizeable = requires(T& obj) { 
        { obj.size() } -> std::convertible_to<size_t>;
    };

    template <typename T>
    concept BigType = requires {
        requires sizeof(T) > 8;
    };

    template <typename T>
    concept AdditiveRange = requires(T&& c) {
        std::ranges::begin(c);                                         // simple requirement
        std::ranges::end(c);                                           // simple requirement
        typename std::ranges::range_value_t<T>;                        // type requirement
        requires requires(std::ranges::range_value_t<T> x) { x + x; }; // nested requirement
    };

    template <AdditiveRange Rng>
    auto sum(const Rng& data)
    {
        return std::accumulate(std::begin(data), std::end(data),
            std::ranges::range_value_t<Rng>{});
    }

} // namespace Training

static_assert(!Training::BigType<char>);
static_assert(Training::BigType<std::vector<int>>);

TEST_CASE("concepts & constriants")
{
    Training::Integral auto result = Training::add(4, 5);

    // Training::Integral auto other_result = Training::add(4.0, 5.7);

    static_assert(Training::Hashable<std::string>);
    // static_assert(Training::Hashable<std::vector<int>>);

    std::vector<int> vec = {1, 2, 3};
    Training::print(vec, "vec");

    // std::map<int, std::string> dict = { {1, "one"} };
    // Training::print(dict, "dict");

    Training::Wrapper w1{42};
    w1.print();

    Training::Wrapper w2{std::vector{1, 2, 3}};
    w2.print();
}

template <typename TItem>
void add_to_container(auto& container, TItem&& item)
{
    if constexpr(requires { container.push_back(std::forward<TItem>(item)); })
        container.push_back(std::forward<TItem>(item));
    else
        container.insert(std::forward<TItem>(item));
}

TEST_CASE("add to container")
{
    std::vector<int> vec = {1, 2, 3};
    add_to_container(vec, 4);

    std::set<int> my_set = {1, 2, 3};
    add_to_container(my_set, 4);
}

//////////////////////////////////////////////
// concept subsumation

struct BoundingBox
{
    int w, h;
};

struct Color
{
    uint8_t r, g, b;
};

template <typename T>
concept Shape = requires(const T& obj)
{
    { obj.box() } noexcept -> std::same_as<BoundingBox>;
    obj.draw();
};

template <typename T>
concept ShapeWithColor = Shape<T> && requires(T shp, Color c) {
    shp.set_color(c);
    { shp.get_color() } -> std::same_as<Color>;
};

struct Rect
{
    int w, h;
    Color color;

    void draw() const
    {
        std::cout << "Rect::draw()\n";
    }

    BoundingBox box() const noexcept
    {
        return BoundingBox{w, h};
    }

    void set_color(Color c)
    {
        std::cout << "Setting color\n";
        color = c;
    }

    Color get_color() const
    {
        return color;
    }
};

static_assert(Shape<Rect>);

template <Shape T>
void render(const T& shp)
{
    shp.draw();
}

template <ShapeWithColor T>
void render(T&& shp)
{
    shp.set_color({0, 0, 0});
    shp.draw();
}

TEST_CASE("subsuming concepts")
{
    Rect rect{10, 20};
    render(rect);
}