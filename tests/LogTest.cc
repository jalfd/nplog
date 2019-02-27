#include <nplog/ScopedMessage.hpp>
#include <catch/catch.hpp>

TEST_CASE("Log") {
    np::Log log;
    REQUIRE(log.suppressMessage(10));
    REQUIRE(log.paramLevel() == 0);
}
