#include <nplog/common.hpp>
#include <catch/catch.hpp>

TEST_CASE("Testing log levels") {
  SECTION("Pass if level is exactly what is configured") { CHECK(np::log::testLevel(5, 5)); }
  SECTION("Pass if level is lower than what is configured") { CHECK(np::log::testLevel(2, 5)); }
  SECTION("Reject if level is higher than what is configured") { CHECK(!np::log::testLevel(6, 5)); }
}
// TODO: need a test that sensitive flag works, here or in another test suite
