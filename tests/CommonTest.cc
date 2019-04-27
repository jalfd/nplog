#include <nplog/common.hpp>
#include <catch/catch.hpp>

TEST_CASE("Testing log levels") {
  SECTION("Pass if level is exactly what is configured") { CHECK(np::testLevel(5, 5)); }
  SECTION("Pass if level is lower than what is configured") { CHECK(np::testLevel(2, 5)); }
  SECTION("Reject if level is higher than what is configured") { CHECK(!np::testLevel(6, 5)); }
}
TEST_CASE("Testing custom user levels") {
  SECTION("If a user level is enabled, messages without still pass") {
    CHECK(np::testLevel(5, 0x0105));
  }
  SECTION("If a user level is enabled, messages with the same user level pass") {
    CHECK(np::testLevel(0x0105, 0x0105));
  }
  SECTION("If a user level is enabled, messages with other user levels fail") {
    CHECK(!np::testLevel(0x0105, 0x0205));
  }
  SECTION("Multiple user levels can be configured") {
    CHECK(np::testLevel(0x0105, 0x0305));
    CHECK(np::testLevel(0x0205, 0x0305));
    CHECK(np::testLevel(0x0305, 0x0305));
  }
}
