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

TEST_CASE("Suppressing message props") {
  SECTION("When a prop level is explicitly specified, it is used to filter the prop") {
    CHECK(!suppressProp({threshold(Status), threshold(Warning)}, Levels(), Error));
    CHECK(!suppressProp({threshold(Status), threshold(Warning)}, Levels(), Warning));
    CHECK(suppressProp({threshold(Status), threshold(Warning)}, Levels(), Status));
  }
  SECTION("When a prop level is not specified, the level of the message is used instead") {
    CHECK(!suppressProp({threshold(Status), threshold(Warning)}, Error, ""));
    CHECK(!suppressProp({threshold(Status), threshold(Warning)}, Warning, ""));
    CHECK(suppressProp({threshold(Status), threshold(Warning)}, Status, ""));
  }
}
