#include <nplog/logger.hpp>
#include <picojson/picojson.h>
#include <catch/catch.hpp>

namespace pj = picojson;

pj::object parseLogMessage(np::log::MessageBuffer buf) {
  pj::value val;
  std::string err;
  const auto contents = buf.contents();
  picojson::parse(val, contents.begin(), contents.end(), &err);
  CAPTURE(contents);
  REQUIRE(err == "");
  REQUIRE(val.is<pj::object>());
  return val.get<pj::object>();
}

TEST_CASE("LogParams") {
  REQUIRE(false);
  SECTION("So a plain logger should not add logparams") {
      np::Logger log;
  }

  SECTION("A logger with logparams should add them to messages") {
  }

  SECTION("A child logger whose parent has logparams should add them to messages") {
  }

  SECTION("A child logger whose ancestor has logparams should add them to messages") {
  }

  SECTION("A child logger with logparams should merge them with those from ancestors") {
  }
}
