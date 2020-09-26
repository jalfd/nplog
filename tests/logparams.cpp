#include <nplog/logger.hpp>
#include <nplog/scopedmessage.hpp>
#include <picojson/picojson.h>
#include <catch/catch.hpp>

namespace pj = picojson;

static pj::object parseMessage(std::string_view contents) {
  pj::value val;
  std::string err;
  picojson::parse(val, contents.begin(), contents.end(), &err);
  REQUIRE(err == "");
  REQUIRE(val.is<pj::object>());
  return val.get<pj::object>();
}

TEST_CASE("LogParams") {
  SECTION("A logger specifying no static params should not allocate a params buffer") {
    np::Logger log;
    REQUIRE(log.loggerParams() == nullptr);
  }

  SECTION("A logger with empty logparams should not allocate a params buffer") {
    np::Logger log(nullptr, nullptr, {});
    REQUIRE(log.loggerParams() == nullptr);
  }

  SECTION("A logger with logparams stores them for messages") {
    np::Logger log(nullptr, nullptr, {{"foo", 42}});
    const auto& params_buffer = log.loggerParams()->data;
    std::string str(params_buffer.contents());
    str = "{" + str + "}";
    const auto msg = parseMessage(str);
    REQUIRE(msg.at("foo") == pj::value(42.0));
  }

  SECTION("A logger with multiple logparams stores them for messages") {
    np::Logger log(nullptr, nullptr, {{"foo", 42}, {"bar", 43}});
    const auto& params_buffer = log.loggerParams()->data;
    std::string str(params_buffer.contents());
    str = "{" + str + "}";
    const auto msg = parseMessage(str);
    REQUIRE(msg.at("foo") == pj::value(42.0));
    REQUIRE(msg.at("bar") == pj::value(43.0));
  }

  SECTION("A child logger whose parent has logparams stores them for messages") {
    np::Logger rootlog(nullptr, nullptr, {{"foo", 42}});
    np::Logger childlog(&rootlog);

    REQUIRE(childlog.loggerParams() != nullptr);
    const auto& params_buffer = childlog.loggerParams()->data;
    std::string str(params_buffer.contents());
    str = "{" + str + "}";
    const auto msg = parseMessage(str);
    REQUIRE(msg.at("foo") == pj::value(42.0));
  }

  SECTION("A child logger whose ancestor has logparams stores them for messages") {
    np::Logger rootlog(nullptr, nullptr, {{"foo", 42}});
    np::Logger parentlog(&rootlog);
    np::Logger childlog(&parentlog);

    const auto& params_buffer = childlog.loggerParams()->data;
    std::string str(params_buffer.contents());
    str = "{" + str + "}";
    const auto msg = parseMessage(str);
    REQUIRE(msg.at("foo") == pj::value(42.0));
  }

  SECTION("A child logger with logparams should merge them with those from ancestors") {
    np::Logger rootlog(nullptr, nullptr, {{"foo", 42}});
    np::Logger childlog(&rootlog, nullptr, {{"bar", std::string("value")}});

    const auto& params_buffer = childlog.loggerParams()->data;
    std::string str(params_buffer.contents());
    str = "{" + str + "}";
    const auto msg = parseMessage(str);
    REQUIRE(msg.at("foo") == pj::value(42.0));
    REQUIRE(msg.at("bar") == pj::value("value"));
  }

  SECTION("Multiple logparams are merged correctly") {
    np::Logger rootlog(nullptr, nullptr, {{"a", 42}, {"b", 43}});
    np::Logger childlog(
      &rootlog, nullptr, {{"c", std::string("value0")}, {"d", std::string("value1")}});

    const auto& params_buffer = childlog.loggerParams()->data;
    std::string str(params_buffer.contents());
    str = "{" + str + "}";
    const auto msg = parseMessage(str);
    REQUIRE(msg.at("a") == pj::value(42.0));
    REQUIRE(msg.at("b") == pj::value(43.0));
    REQUIRE(msg.at("c") == pj::value("value0"));
    REQUIRE(msg.at("d") == pj::value("value1"));
  }

  SECTION("Child logger logparams take precedence") {
    np::Logger rootlog(nullptr, nullptr, {{"foo", 42}});
    np::Logger childlog(&rootlog, nullptr, {{"foo", std::string("value")}});

    const auto& params_buffer = childlog.loggerParams()->data;
    std::string str(params_buffer.contents());
    str = "{" + str + "}";
    const auto msg = parseMessage(str);
    REQUIRE(msg.at("foo") == pj::value("value"));
  }
}

TEST_CASE("Messages include the loggers logparams") {
    std::string out;
  np::log::Config cfg;
  cfg.fields = static_cast<np::log::Config::Fields>(0);
  cfg.sink = [&](const np::log::MessageInfo& mi) {
      out = mi.message;
  };
  np::log::applyConfig(cfg);
  np::Logger log(nullptr, nullptr, {{"foo", 42}});
  { np::log::ScopedMessage msg(log, "", 0, 0, "", 0); }
  auto msg = parseMessage(out);
  REQUIRE(msg.at("static").get<pj::object>().at("foo") == pj::value(42.0));
}
