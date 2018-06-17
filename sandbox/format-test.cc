#include <catch.hpp>

// The function under test
template <typename OutIter, typename ArgType>
OutIter format(OutIter dest, ArgType arg);


// wrapper to make it easily testable

template <typename OutIter, typename ArgType>
auto run_format(OutIter dest, ArgType arg) {
    std::string out;
    format(std::back_inserter(dest), arg);
    return out;
}

struct UDT {};

TEST_CASE("boo") {
    REQUIRE(run_format(true) == "true");
    REQUIRE(run_format("hello") == "hello");
    REQUIRE(run_format(42) == "42");
    REQUIRE(run_format(0.42) == "0.42");
    REQUIRE(run_format(UDT{}) == "{\"foo\":\"bar\"}");

}
