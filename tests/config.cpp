#include <nplog/macros.hpp>
#include <nplog/scopedmessage.hpp>
#include <picojson/picojson.h>
#include "../src/configimpl.hpp"
#include <catch/catch.hpp>

namespace pj = picojson;

int msg_count = 0;

bool isLogged(np::log::LogGroup& log, int level) {
  msg_count = 0;
  NP_LOG(log, level, "message text");
  return msg_count != 0;
}

TEST_CASE("Configuring Log Default Levels") {
  np::log::Config cfg;
  cfg.sink = [&](auto) mutable { ++msg_count; };
  np::log::applyConfig(cfg);
  auto lvls = np::log::getLevels("", 0);
  CHECK(np::log::LevelWrapper(lvls.effective_levels.message).l == np::log::threshold(np::log::Status).l);
  CHECK(np::log::LevelWrapper(lvls.effective_levels.param).l == np::log::threshold(np::log::Status).l);
  auto prev_level = lvls.version;

  cfg.levels.default_level = {threshold(np::log::Fatal), threshold(np::log::Fatal)};
  np::log::applyConfig(cfg);

  SECTION("Setting new log levels") {
    auto lvls = np::log::getLevels("", 0);
    CHECK(lvls.version > prev_level);
    CHECK(lvls.effective_levels.message == np::log::Fatal);
    CHECK(lvls.effective_levels.param == np::log::Fatal);

    prev_level = lvls.version;

    cfg.levels.default_level = {threshold(np::log::Error), threshold(np::log::Warning)};
    np::log::applyConfig(cfg);

    SECTION("Resetting log levels") {
      auto lvls = np::log::getLevels("", 0);
      CHECK(lvls.version > prev_level);
      CHECK(lvls.effective_levels.message == threshold(np::log::Error).l);
      CHECK(lvls.effective_levels.param == threshold(np::log::Warning).l);
    }
  }
}

TEST_CASE("Configuring Log Levels by depth") {
  np::log::Config cfg;
  cfg.sink = [&](auto) mutable { ++msg_count; };
  np::log::applyConfig(cfg);
  cfg.levels.default_level = {threshold(np::log::Fatal), threshold(np::log::Fatal)};
  cfg.levels.levels_by_depth[0] = {threshold(np::log::DebugLow), threshold(np::log::DebugLow)};
  cfg.levels.levels_by_depth[1] = {threshold(np::log::Status), threshold(np::log::Status)};
  cfg.levels.levels_by_depth[3] = {threshold(np::log::Warning), threshold(np::log::Warning)};
  np::log::applyConfig(cfg);

  // For depths 0 and 1, use the specified rules
  CHECK(np::log::getLevels("", 0).effective_levels.message == threshold(np::log::DebugLow).l);
  CHECK(np::log::getLevels("", 0).effective_levels.param == threshold(np::log::DebugLow).l);
  CHECK(np::log::getLevels("", 1).effective_levels.message == threshold(np::log::Status).l);
  CHECK(np::log::getLevels("", 1).effective_levels.param == threshold(np::log::Status).l);
  // We didn't specify a rules for depth 2.
  // Fallback to the rule for the next higher level (3)
  CHECK(np::log::getLevels("", 2).effective_levels.message == threshold(np::log::Warning).l);
  CHECK(np::log::getLevels("", 2).effective_levels.param == threshold(np::log::Warning).l);
  // We only specified special rules for depths up to 3.
  // Fall back to default once depth exceeds the
  CHECK(np::log::getLevels("", 4).effective_levels.message == threshold(np::log::Fatal).l);
  CHECK(np::log::getLevels("", 4).effective_levels.param == threshold(np::log::Fatal).l);

  SECTION("Level based on depth is not inherited") {
    np::log::LogGroup log0; // depth 0 -> log level Fatal
    np::log::LogGroup log1(&log0); // depth 1 -> log level Status

    CHECK(!isLogged(log1, np::log::DebugHigh));
  }
}

TEST_CASE("Configuring Log Levels by log name") {
  np::log::Config cfg;
  cfg.sink = [&](auto) mutable { ++msg_count; };
  cfg.levels.default_level = {threshold(np::log::Fatal), threshold(np::log::Fatal)};
  cfg.levels.levels_by_name["foo"] = {threshold(np::log::DebugLow), threshold(np::log::DebugLow)};
  cfg.levels.levels_by_name["bar"] = {threshold(np::log::Status), threshold(np::log::Status)};
  np::log::applyConfig(cfg);

  // Name not found. Fall back to defaults
  CHECK(np::log::getLevels("xyz", 0).effective_levels.message == np::log::Fatal);
  CHECK(np::log::getLevels("xyz", 0).effective_levels.param == np::log::Fatal);
  // Use level specified for the given name
  CHECK(np::log::getLevels("foo", 0).effective_levels.message == threshold(np::log::DebugLow).l);
  CHECK(np::log::getLevels("foo", 0).effective_levels.param == threshold(np::log::DebugLow).l);

  SECTION("Level based on name is inherited") {
    np::log::LogGroup log0("foo"); // name 'foo' -> log level DebugLow
    np::log::LogGroup log1(&log0, "x"); // inherits level DebugLow from ancestor
    np::log::LogGroup log2(&log1, "y"); // inherits level DebugLow transitively as well
    np::log::LogGroup log3(
      &log0, "bar"); // inherits level DebugLow from ancestor, gets level Status from own name
    CHECK(isLogged(log1, np::log::DebugLow));
    CHECK(isLogged(log2, np::log::DebugLow));
    CHECK(isLogged(log3, np::log::DebugLow));
  }
}

TEST_CASE("Prioritizing log levels when both level and name rules apply") {
  np::log::Config cfg;
  cfg.sink = [&](auto) mutable { ++msg_count; };
  cfg.levels.default_level = {threshold(np::log::Fatal), threshold(np::log::Fatal)};
  cfg.levels.levels_by_name["foo"] = {threshold(np::log::DebugLow), threshold(np::log::DebugLow)};
  cfg.levels.levels_by_name["bar"] = {threshold(np::log::Status), threshold(np::log::Status)};

  cfg.levels.levels_by_depth[0] = {threshold(np::log::DebugHigh), threshold(np::log::DebugHigh)};
  cfg.levels.levels_by_depth[1] = {threshold(np::log::Warning), threshold(np::log::Warning)};
  np::log::applyConfig(cfg);

  // matches both lvl0 -> DebugHigh and name -> DebugLow
  np::log::LogGroup log0("foo");
  CHECK(isLogged(log0, np::log::DebugLow));

  // matches both lvl0 -> DebugHigh and name -> Status
  np::log::LogGroup log1("bar");
  CHECK(isLogged(log1, np::log::DebugHigh));

  // matches both lvl0 -> DebugHigh and name -> Status and inherited name -> DebugLow
  np::log::LogGroup log2(&log0, "bar");
  CHECK(isLogged(log0, np::log::DebugLow));
}

TEST_CASE("Header fields can be toggled on and off") {
  np::log::Config cfg;
  std::string err;
  pj::object result;
  cfg.fields = static_cast<np::log::Fields>(0);
  cfg.sink = [&](auto msg) mutable {
    pj::value val;
    pj::parse(val, msg.message.begin(), msg.message.end(), &err);
    result = val.get<pj::object>();
    REQUIRE(err.empty());
  };

  np::log::LogGroup log("myname");
  ;

  SECTION("If all fields are disabled, only message is logged") {
    np::log::applyConfig(cfg);
    NP_LOG(log, 0, "dummy message");
    CAPTURE(result);
    REQUIRE(result.size() == 1);
    REQUIRE(result["message"].get<std::string>() == "dummy message");
  }

  SECTION("Add File field") {
    cfg.fields = np::log::File;
    np::log::applyConfig(cfg);
    NP_LOG(log, 0, "dummy message");
    CAPTURE(result);
    REQUIRE(result.size() == 2);
    REQUIRE(result.find("file") != result.end());
  }

  SECTION("Add Line field") {
    cfg.fields = np::log::Line;
    np::log::applyConfig(cfg);
    NP_LOG(log, 0, "dummy message");
    CAPTURE(result);
    REQUIRE(result.size() == 2);
    REQUIRE(result.find("line") != result.end());
  }
  SECTION("Add Time field") {
    cfg.fields = np::log::Time;
    np::log::applyConfig(cfg);
    NP_LOG(log, 0, "dummy message");
    CAPTURE(result);
    REQUIRE(result.size() == 2);
    REQUIRE(result.find("time") != result.end());
  }
  SECTION("Add Level field") {
    cfg.fields = np::log::Level;
    np::log::applyConfig(cfg);
    NP_LOG(log, 0, "dummy message");
    CAPTURE(result);
    REQUIRE(result.size() == 2);
    REQUIRE(result.find("level") != result.end());
  }
  SECTION("Add LevelName field") {
    cfg.fields = np::log::LevelName;
    np::log::applyConfig(cfg);
    NP_LOG(log, 0, "dummy message");
    CAPTURE(result);
    REQUIRE(result.size() == 2);
    REQUIRE(result.find("levelString") != result.end());
  }
  SECTION("Add LogName field") {
    cfg.fields = np::log::LogName;
    np::log::applyConfig(cfg);
    NP_LOG(log, 0, "dummy message");
    CAPTURE(result);
    REQUIRE(result.size() == 2);
    REQUIRE(result.find("log") != result.end());
  }
  SECTION("Add ProcessName field") {
    cfg.fields = np::log::ProcessName;
    np::log::applyConfig(cfg);
    NP_LOG(log, 0, "dummy message");
    CAPTURE(result);
    REQUIRE(result.size() == 2);
    REQUIRE(result.find("process") != result.end());
  }
  SECTION("Add ProcessId field") {
    cfg.fields = np::log::ProcessId;
    np::log::applyConfig(cfg);
    NP_LOG(log, 0, "dummy message");
    CAPTURE(result);
    REQUIRE(result.size() == 2);
    REQUIRE(result.find("pid") != result.end());
  }
  SECTION("Add ThreadId field") {
    cfg.fields = np::log::ThreadId;
    np::log::applyConfig(cfg);
    NP_LOG(log, 0, "dummy message");
    CAPTURE(result);
    REQUIRE(result.size() == 2);
    REQUIRE(result.find("tid") != result.end());
  }
  SECTION("Add Hostname field") {
    cfg.fields = np::log::Hostname;
    np::log::applyConfig(cfg);
    NP_LOG(log, 0, "dummy message");
    CAPTURE(result);
    REQUIRE(result.size() == 2);
    REQUIRE(result.find("host") != result.end());
  }
}
