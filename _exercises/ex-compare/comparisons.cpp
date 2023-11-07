#include <catch2/catch_test_macros.hpp>
#include <format>
#include <iostream>
#include <string>
#include <vector>

using namespace std::literals;

struct Rating
{
    int value;

    bool operator==(const Rating& other) const
    {
        return value == other.value;
    }

    bool operator<(const Rating& other) const
    {
        return value < other.value;
    }
};

struct Gadget
{
    std::string name;
    double price;

    auto operator<=>(const Gadget& other) const = default;
};

struct SuperGadget : Gadget
{
    Rating rating;

    std::partial_ordering operator<=>(const SuperGadget& other) const = default;
};

namespace SaferComparison
{
    struct Gadget
    {
        std::string name;
        double price;

        bool operator==(const Gadget&) const = default;

        std::strong_ordering operator<=>(const Gadget& other) const
        {
            if (auto order = name <=> other.name; order == 0)
            {
                return std::strong_order(price, other.price);
            }
            else
                return order;
        }
    };

    struct SuperGadget : Gadget
    {
        Rating rating;

        std::strong_ordering operator<=>(const SuperGadget&) const = default;
    };
} // namespace SaferComparison

TEST_CASE("Gadget - write custom operator <=> - stronger category than auto detected")
{
    SECTION("==")
    {
        CHECK(Gadget{"ipad", 1.0} == Gadget{"ipad", 1.0});
    }

    SECTION("<=>")
    {
        CHECK(Gadget{"ipad", 1.0} <=> Gadget{"ipad", 1.0} == std::strong_ordering::equal);
    }
}

TEST_CASE("SuperGadget - write custom operator <=> - member without compare-three-way operator")
{
    CHECK(SuperGadget{{"ipad", 1.0}, Rating{1}} != SuperGadget{{"ipad", 1.0}, Rating{2}});
    CHECK(SuperGadget{{"ipad", 1.0}, Rating{1}} <=> SuperGadget{{"ipad", 1.0}, Rating{2}} == std::strong_ordering::less);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

enum class RatingValue : uint8_t { very_poor = 1,
    poor,
    satisfactory,
    good,
    very_good,
    excellent };

struct RatingStar
{
public:
    RatingValue value;

    explicit RatingStar(RatingValue rating_value)
        : value{rating_value}
    { }

    auto operator<=>(const RatingStar& other) const = default;

    auto operator<=>(const RatingValue& other) const
    {
        // return *this<=>RatingStar(other);
        return value <=> other;
    }
};

TEST_CASE("Rating Star - implement nedded <=>")
{
    RatingStar r1{RatingValue::good};

    CHECK(r1 == RatingStar{RatingValue::good});
    CHECK(r1 <=> RatingStar{RatingValue::excellent} == std::strong_ordering::less);
    CHECK(r1 <=> RatingValue::excellent == std::strong_ordering::less);
}