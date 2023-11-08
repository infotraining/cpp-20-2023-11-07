#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <helpers.hpp>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>
#include <list>
#include <map>

using namespace std::literals;

void print(std::ranges::range auto&& rng, std::string_view name)
{
    std::cout << name << ": [ ";
    for (const auto& item : rng)
        std::cout << item << " ";
    std::cout << "]\n";
}

TEST_CASE("ranges")
{
    auto data = helpers::create_numeric_dataset<100>(42);
    print(data, "data");
}

template <auto Value_>
struct EndValue
{
    bool operator==(std::input_or_output_iterator auto pos) const
    {
        return *pos == Value_;
    }
};

TEST_CASE("ranges - algorithms")
{
    SECTION("basics")
    {
        auto data = helpers::create_numeric_dataset<100>(42);

        std::ranges::sort(data);
        print(data, "data");

        std::vector<int> positive_numbers;
        std::ranges::copy_if(data, std::back_inserter(positive_numbers), [](int n) { return n > 0; });
        print(positive_numbers, "positive_numbers");
    }

    SECTION("projections")
    {
        std::vector<std::string> words = {"twenty-two"s, "a"s, "abc"s, "b"s, "one"s, "aa"s};

        //std::sort(words.begin(), words.end(), [](const auto& s1, const auto& s2) { return s1.size() < s2.size(); });
        std::ranges::sort(words, std::less{}, /*projection*/ [](const auto& s) { return s.size(); });

        print(words, "words sorted by size");
    }

    SECTION("sentinels")
    {
        std::string str = "fajsdkh.gjadfg";

        std::ranges::sort(str.begin(), EndValue<'.'>{});

        print(str, "str");

        EndValue<'\0'> null_term;
        
        auto& txt = "acbgdef\0ajdhfgajsdhfgkasdjhfg"; // const char(&txt)[40]
        std::array txt_array = std::to_array(txt);

        std::ranges::sort(txt_array.begin(), null_term, std::greater{});

        print(txt_array, "txt_after sort");

        std::vector data = {5, 423, 665, 1, 235, 42, 6, 345, 33, 665};        

        auto pos = std::ranges::find(data.begin(), std::unreachable_sentinel, 42);
        CHECK(*pos == 42);

        for(auto it = std::counted_iterator{data.begin(), 5}; it != std::default_sentinel; ++it)
        {
            std::cout << *it << " ";
        }
        std::cout << "\n";

        std::vector<int> items1(5);
        std::ranges::copy(std::span{data.begin(), 5}, items1.begin());

        std::vector<int> items2(5);
        std::ranges::copy(std::counted_iterator{data.begin(), 5}, std::default_sentinel, items2.begin());

        CHECK(items1 == items2);
    }
}

TEST_CASE("ranges - views")
{
    std::list lst = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9, 10};

    SECTION("all")
    {
        auto all_items = std::views::all(lst);

        print(all_items, "all items");

        auto all_items_view = all_items;
    }

    SECTION("counted")
    {
        auto first_half = std::views::counted(lst.begin(), lst.size() / 2);

        for(auto& item : first_half)
            item *= 2;

        print(lst, "lst");
    }

    SECTION("iota")
    {
        auto my_range = std::views::iota(1);

        print(my_range | std::views::take(20), "my_range");
    }

    SECTION("piping")
    {
        auto data = std::views::iota(1)
                        | std::views::take(20)
                        | std::views::filter([](int x) { return x % 2 == 0;})
                        | std::views::transform([](int x) { return x * x;})
                        | std::views::reverse;

        auto other_view = data; // O(1)

        print(other_view, "data");
    }

    SECTION("keys - values")
    {
        std::map<int, std::string> dict = { {1, "one"}, {2, "two"} };

        print(dict | std::views::keys, "keys");
        print(dict | std::views::values, "value");
    }
}


std::vector<std::string_view> tokenize(std::string_view text, auto token)
{
    auto tokens = text | std::views::split(token);

    std::vector<std::string_view> tokens_sv;
    
    for(auto&& rng : tokens)
    {
        tokens_sv.push_back(std::string_view(&(*rng.begin()), rng.end() - rng.begin()));
    }

    return tokens_sv;
}

TEST_CASE("split")
{
    std::string str = "abc,def,ghi";

    std::vector<std::string_view> tokens_sv = tokenize(str, ',');
    
    print(tokens_sv, "tokens_sv");
}