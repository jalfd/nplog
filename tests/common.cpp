#include <nplog/common.hpp>
#include <catch/catch.hpp>

using namespace np::log;

TEST_CASE("Testing log levels") {
  SECTION("Pass if level is exactly what is configured") {
    CHECK(np::log::testLevel(Fatal, Fatal));
  }
  SECTION("Pass if the indicated level is among those configured") {
    CHECK(np::log::testLevel(Error, Fatal | Error | Warning));
  }
  SECTION("Reject if level is not among those configured") {
    CHECK(!np::log::testLevel(Status, Fatal | Error | Warning));
  }
  SECTION("Reject if not all levels are not among those configured") {
    CHECK(!np::log::testLevel(Status | 1024, Fatal | Error | Warning | Status));
  }
  SECTION("Pass if all levels are not among those configured") {
    CHECK(np::log::testLevel(Status | 1024, Fatal | Error | Warning | Status | 1024));
  }
}

TEST_CASE("Suppressing message params") {
  SECTION("When a param level is explicitly specified, it is used to filter the param") {
    CHECK(!suppressParam({4, 3}, 0, 2));
    CHECK(!suppressParam({4, 3}, 0, 3));
    CHECK(suppressParam({4, 3}, 0, 4));
  }
  SECTION("When a param level is not specified, the level of the message is used instead") {
    CHECK(!suppressParam({4, 3}, 2, ""));
    CHECK(!suppressParam({4, 3}, 3, ""));
    CHECK(suppressParam({4, 3}, 4, ""));
  }
}
