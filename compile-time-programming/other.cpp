#include <catch2/catch_test_macros.hpp>

extern int id;

int global_id = id + 1;

TEST_CASE("global_id")
{
    CHECK(global_id == 666);
}