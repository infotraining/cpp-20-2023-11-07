#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <list>
#include <mutex>
#include <optional>
#include <string>
#include <vector>
#include <source_location>

using namespace std::literals;

[[nodiscard("Always check if data was loaded")]] std::optional<std::vector<int>> load_data(const std::string& path)
{
    if (path == "")
    {
        return std::nullopt;
    }

    return std::vector{1, 2, 3};
}

TEST_CASE("range-based for with init")
{
    // SECTION("UB")
    // {
    //     for (const auto& item : load_data("some_data.dat").value())
    //     {
    //         std::cout << item << " ";
    //     }
    //     std::cout << "\n";
    // }

    SECTION("temporary object - safe iteration")
    {
        for (auto&& opt_data = load_data("some_data.dat"); const auto& item : opt_data.value())
        {
            std::cout << item << " ";
        }
        std::cout << "\n";
    }

    SECTION("mutex + data")
    {
        static std::vector vec = {1, 2, 3};
        static std::mutex mtx_vec;

        // #THD
        for (std::scoped_lock lk{mtx_vec}; auto& item : vec)
        {
            item *= 2;
        }

        std::list<int> lst = {1, 2, 3};

        for (auto [it, index] = std::tuple{lst.begin(), 0}; it != lst.end(); ++it, ++index)
        {
            std::cout << index << "th: " << *it << "\n";
        }
    }
}

enum class DayOfWeek { mon, tue, wed, thd, fri, sat, sun };

TEST_CASE("enum class + using")
{
    DayOfWeek today = DayOfWeek::thd;

    using enum DayOfWeek;

    DayOfWeek tomorrow = fri;

    load_data("");
}

struct Empty  // sizof(Empty) == 1
{};

static_assert(sizeof(Empty) == 1); 

struct Value // sizeof(Valueue) == 4
{
    int i;
};

static_assert(sizeof(Value) == 4); 
 
struct EmptyAndValue // sizeof(EmptyAndValue) == 8
{
    Empty e;
    int i;
};

static_assert(sizeof(EmptyAndValue) == 8);

 
struct EmptyWithAttrAndValue // sizeof(EmptyWithAttrAndValue) == 4
{
    [[no_unique_address]] Empty e;
    int i;
};

static_assert(sizeof(EmptyWithAttrAndValue) == 4);

struct ValueAndEmptyWithAttr // sizeof(ValueueAndEmptyWithAttr) == 4
{
    int i;
    [[no_unique_address]] Empty e;
};

static_assert(sizeof(ValueAndEmptyWithAttr) == 4);

TEST_CASE("UTF-8")
{
    std::u8string str_utf8 = u8"Krak\u00F3w";
    const char8_t* cstr_utf8 = u8"Krak\u00F3w";
    auto sv_utf8 = u8"Gda\u0144sk"sv;

    std::cout << reinterpret_cast<const char*>(str_utf8.data()) << "\n";

    auto sloc = std::source_location::current();

    std::cout << "file: " << sloc.file_name() << " - line: " << sloc.line() << " - func: " << sloc.function_name() << "\n";
}