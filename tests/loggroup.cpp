#include "../src/messagebuffer.hpp"
#include <nplog/scopedmessage.hpp>
#include <catch2/catch_test_macros.hpp>
#include <nplog/config.hpp>
#include <nplog/macros.hpp>
#include <optional>
#include <picojson/picojson.h>

namespace pj = picojson;

static pj::object parseMessage(std::string_view contents) {
  pj::value val;
  std::string err;
  picojson::parse(val, contents.begin(), contents.end(), &err);
  CAPTURE(contents);
  REQUIRE(err == "");
  REQUIRE(val.is<pj::object>());
  return val.get<pj::object>();
}
TEST_CASE("LogGroup") {
  SECTION("Messages include the loggroup name when appropriate") {
    np::log::Config cfg;
    cfg.fields = np::log::LogName;
    cfg.levels.default_level = {threshold(np::log::Status)};

    std::optional<std::string> logged;
    cfg.sink = [&](const auto& msg) { logged = msg.message; };
    np::log::applyConfig(cfg);

    SECTION("LogGroup has no name") {
      np::log::LogGroup lg;
      LOG(lg, np::log::Status, "");
      REQUIRE(logged);
      const auto parsed = parseMessage(*logged);
      REQUIRE(parsed.find("log") == parsed.end());
    }

    SECTION("LogGroup is named") {
      np::log::LogGroup lg("somename");
      LOG(lg, np::log::Status, "");
      REQUIRE(logged);
      const auto parsed = parseMessage(*logged);
      REQUIRE(parsed.at("log").get<std::string>() == "somename");
    }

    SECTION("LogGroup is named, but the name field is disabled") {
      cfg.fields = {};
      np::log::applyConfig(cfg);
      np::log::LogGroup lg("somename");
      LOG(lg, np::log::Status, "");
      REQUIRE(logged);
      const auto parsed = parseMessage(*logged);
      REQUIRE(parsed.find("log") == parsed.end());
    }
  }
}

