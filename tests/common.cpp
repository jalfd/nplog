#include <nplog/common.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace np::log;

TEST_CASE("Testing log levels") {
  SECTION("Pass if level is equal to configured threshold") {
    CHECK(np::log::testLevel(Fatal, Fatal));
  }
  SECTION("Pass if the level is more severe than configured threshold") {
    CHECK(np::log::testLevel(Error, Warning));
  }
  SECTION("Reject if level less severe than configured threshold") {
    CHECK(!np::log::testLevel(Info, Warning));
  }
  SECTION("Threshold uses custom granularity") {
    CHECK(np::log::testLevel(Warning, Info + 2));
    CHECK(np::log::testLevel(Info, Info + 2));
    CHECK(np::log::testLevel(Info + 2, Info + 2));
    CHECK(!np::log::testLevel(Info + 3, Info + 2));
    CHECK(!np::log::testLevel(Debug, Info + 2));
  }
}

TEST_CASE("Suppressing message props") {
  SECTION("When a prop level is explicitly specified, it is used to filter the prop") {
    CHECK(!suppressProp({Info, Warning}, Levels(), Error));
    CHECK(!suppressProp({Info, Warning}, Levels(), Warning));
    CHECK(suppressProp({Info, Warning}, Levels(), Info));
  }
  SECTION("When a prop level is not specified, the level of the message is used instead") {
    CHECK(!suppressProp({Info, Warning}, Error, ""));
    CHECK(!suppressProp({Info, Warning}, Warning, ""));
    CHECK(suppressProp({Info, Warning}, Info, ""));
  }
}
