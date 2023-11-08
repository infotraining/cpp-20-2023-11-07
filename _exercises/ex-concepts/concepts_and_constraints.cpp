#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <source_location>
#include <string>
#include <unordered_map>
#include <vector>
#include <concepts>

using namespace std::literals;

/*********************
Iterator concept
1. iterator is derefencable: *iter
2. can be pre-incremented - returns reference to iterator
3. can be postincremented
4. can equiality comparable: supports == and !=
**********************/

/*********************
StdContainer concept
1. std::begin(C&) returns iterator
2. std::end(C&) returns iterator
3. std::size(C&) returns T convertible to size_t
**********************/

/*********************
IndexableContainer concept
1. is StdContainer
2. can be indexed: c[index]
**********************/

constexpr static bool TODO = false;

template <typename I>
concept Iterator = requires(I a) {
    *a;
    { ++a } -> std::same_as<I&>;
    a++;
    a == a;
    a != a;
};

namespace AlternativeTake
{
    template <typename I>
    concept Iterator = std::equality_comparable<I> && requires(I a) {
        *a;
        { ++a } -> std::same_as<I&>;
        a++;
    };
}

static_assert(Iterator<int> == false);
static_assert(Iterator<std::vector<int>::iterator>);
static_assert(Iterator<std::vector<int>::const_iterator>);
static_assert(Iterator<int*>);
static_assert(Iterator<const int*>);

template <typename Container>
concept StdContainer = requires(Container& C) {
    { std::begin(C) } -> Iterator;                   // Iterator<decltype(std::begin(C))>
    { std::end(C) } -> Iterator;
    { std::size(C) } -> std::convertible_to<size_t>; // Concept(decltype(std::size(C))) -> std::convertible_to<decltype(std::size(C)), size_t>
};


// trait Index
template <typename T>
struct Index
{
    using type = size_t;
};

template<typename T>
    requires requires { typename T::key_type; }
struct Index<T>
{
    using type = typename T::key_type;
};

template <typename T>
using Index_t = typename Index<T>::type;

template <typename C>
concept Indexable = requires(C obj, Index_t<C> index)
{
    obj[index];
};

template <typename C>
concept IndexableContainer = StdContainer<C> && Indexable<C>;

TEST_CASE("concepts")
{
    static_assert(StdContainer<std::vector<int>>);
    static_assert(StdContainer<std::list<int>>);
    static_assert(StdContainer<std::set<int>>);
    static_assert(StdContainer<std::map<int, std::string>>);
    static_assert(StdContainer<std::unordered_map<int, int>>);
    static_assert(StdContainer<std::vector<bool>>);
    static_assert(StdContainer<std::string>);
    int arr[32];
    static_assert(StdContainer<decltype(arr)>);

    static_assert(IndexableContainer<std::vector<int>>);
    static_assert(!IndexableContainer<std::list<int>>);
    static_assert(!IndexableContainer<std::set<int>>);
    static_assert(IndexableContainer<std::map<int, std::string>>);
    static_assert(IndexableContainer<std::map<std::string, std::string>>);
    static_assert(IndexableContainer<std::unordered_map<int, int>>);
    static_assert(IndexableContainer<std::vector<bool>>);
    static_assert(IndexableContainer<std::string>);
    static_assert(IndexableContainer<decltype(arr)>);
}

void print_all(const StdContainer auto& container)
{
    std::cout << "void print_all(const StdContainer auto& container)\n";

    for (const auto& item : container)
    {
        std::cout << item << " ";
    }
    std::cout << "\n";
}

void print_all(const IndexableContainer auto& container)
{
    std::cout << "void print_all(const IndexableContainer auto& container)\n";

    for (size_t i = 0; i < std::size(container); ++i)
    {
        std::cout << container[i] << " ";
    }
    std::cout << "\n";
}

TEST_CASE("container concepts")
{
    std::vector vec = {1, 2, 3, 4};
    print_all(vec);

    std::list lst{1, 2, 3};
    print_all(lst);
}