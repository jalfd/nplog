#include <nplog/config.hpp>
#include <nplog/log.hpp>
#include <nplog/scopedmessage.hpp>
#include <nplog/macros.hpp>
#include "../src/configimpl.hpp"
#include <catch/catch.hpp>

int msg_count = 0;

bool isLogged(np::log::Log& log, int level) {
  msg_count = 0;
  LOG(log, level, "message text");
  return msg_count != 0;
}

TEST_CASE("Configuring Log Default Levels") {
  np::log::Config cfg;
  cfg.sink = [&](auto) mutable { ++msg_count; };
  np::log::applyConfig(cfg);
  auto lvls = np::log::getLevels("", 0);
  CHECK(lvls.effective_levels.message == 0);
  CHECK(lvls.effective_levels.param == 0);
  auto prev_level = lvls.version;

  cfg.levels.default_level = {1, 1};
  np::log::applyConfig(cfg);

  SECTION("Setting new log levels") {
    auto lvls = np::log::getLevels("", 0);
    CHECK(lvls.version > prev_level);
    CHECK(lvls.effective_levels.message == 1);
    CHECK(lvls.effective_levels.param == 1);

    prev_level = lvls.version;

    cfg.levels.default_level = {2, 3};
    np::log::applyConfig(cfg);

    SECTION("Resetting log levels") {
      auto lvls = np::log::getLevels("", 0);
      CHECK(lvls.version > prev_level);
      CHECK(lvls.effective_levels.message == 2);
      CHECK(lvls.effective_levels.param == 3);
    }
  }
}

TEST_CASE("Configuring Log Levels by depth") {
  np::log::Config cfg;
  cfg.sink = [&](auto) mutable { ++msg_count; };
  np::log::applyConfig(cfg);
  cfg.levels.default_level = {1, 1};
  cfg.levels.levels_by_depth[0] = {9,9};
  cfg.levels.levels_by_depth[1] = {5,5};
  cfg.levels.levels_by_depth[3] = {3,3};
  np::log::applyConfig(cfg);

  // For depths 0 and 1, use the specified rules
  CHECK(np::log::getLevels("", 0).effective_levels.message == 9);
  CHECK(np::log::getLevels("", 0).effective_levels.param == 9);
  CHECK(np::log::getLevels("", 1).effective_levels.message == 5);
  CHECK(np::log::getLevels("", 1).effective_levels.param == 5);
  // We didn't specify a rules for depth 2.
  // Fallback to the rule for the next higher level (3)
  CHECK(np::log::getLevels("", 2).effective_levels.message == 3);
  CHECK(np::log::getLevels("", 2).effective_levels.param == 3);
  // We only specified special rules for depths up to 3.
  // Fall back to default once depth exceeds the
  CHECK(np::log::getLevels("", 4).effective_levels.message == 1);
  CHECK(np::log::getLevels("", 4).effective_levels.param == 1);

  SECTION("Level based on depth is not inherited") {
    np::log::Log log0; // depth 0 -> log level 9
    np::log::Log log1(&log0); // depth 1 -> log level 5

    CHECK(!isLogged(log1, 6));
  }
}

TEST_CASE("Configuring Log Levels by log name") {
  np::log::Config cfg;
  cfg.sink = [&](auto) mutable { ++msg_count; };
  cfg.levels.default_level = {1, 1};
  cfg.levels.levels_by_name["foo"] = {9,9};
  cfg.levels.levels_by_name["bar"] = {5,5};
  np::log::applyConfig(cfg);

  // Name not found. Fall back to defaults
  CHECK(np::log::getLevels("xyz", 0).effective_levels.message == 1);
  CHECK(np::log::getLevels("xyz", 0).effective_levels.param == 1);
  // Use level specified for the given name
  CHECK(np::log::getLevels("foo", 0).effective_levels.message == 9);
  CHECK(np::log::getLevels("foo", 0).effective_levels.param == 9);

  SECTION("Level based on name is inherited") {
    np::log::Log log0("foo"); // name 'foo' -> log level 9
    np::log::Log log1(&log0, "x"); // inherits level 9 from ancestor
    np::log::Log log2(&log1, "y"); // inherits level 9 transitively as well
    np::log::Log log3(&log0, "bar"); // inherits level 9 from ancestor, gets level 5 from own name
    CHECK(isLogged(log1, 9));
    CHECK(isLogged(log2, 9));
    CHECK(isLogged(log3, 9));
  }
}

TEST_CASE("Prioritizing log levels when both level and name rules apply") {
  np::log::Config cfg;
  cfg.sink = [&](auto) mutable { ++msg_count; };
  cfg.levels.default_level = {1, 1};
  cfg.levels.levels_by_name["foo"] = {9,9};
  cfg.levels.levels_by_name["bar"] = {5,5};

  cfg.levels.levels_by_depth[0] = {7,7};
  cfg.levels.levels_by_depth[1] = {3,3};
  np::log::applyConfig(cfg);

  // matches both lvl0 -> 7 and name -> 9
  np::log::Log log0("foo");
  CHECK(isLogged(log0, 9));

  // matches both lvl0 -> 7 and name -> 5
  np::log::Log log1("bar");
  CHECK(isLogged(log1, 7));

  // matches both lvl0 -> 7 and name -> 5 and inherited name -> 9
  np::log::Log log2(&log0, "bar");
  CHECK(isLogged(log0, 9));
}
