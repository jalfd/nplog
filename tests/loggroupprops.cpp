#include <nplog/loggroup.hpp>
#include <nplog/config.hpp>
#include <nplog/scopedmessage.hpp>
#include "../src/loggroupprops.hpp"
#include <picojson/picojson.h>
#include <catch2/catch_test_macros.hpp>

namespace pj = picojson;

static pj::object parseMessage(std::string_view contents) {
  pj::value val;
  std::string err;
  picojson::parse(val, contents.begin(), contents.end(), &err);
  REQUIRE(err == "");
  REQUIRE(val.is<pj::object>());
  return val.get<pj::object>();
}

TEST_CASE("LogProps") {
  SECTION("A logger specifying no static props should not allocate a props buffer") {
    np::LogGroup log;
    REQUIRE(log.props() == nullptr);
  }

  SECTION("A logger with empty logprops should not allocate a props buffer") {
    np::LogGroup log(nullptr, nullptr, {});
    REQUIRE(log.props() == nullptr);
  }

  SECTION("A logger with logprops stores them for messages") {
    np::LogGroup log(nullptr, nullptr, {{"foo", 42}});
    const auto& props_buffer = log.props()->data;
    std::string str(props_buffer.contents());
    str = "{" + str + "}";
    const auto msg = parseMessage(str);
    REQUIRE(msg.at("foo") == pj::value(42.0));
  }

  SECTION("A logger with multiple logprops stores them for messages") {
    np::LogGroup log(nullptr, nullptr, {{"foo", 42}, {"bar", 43}});
    const auto& props_buffer = log.props()->data;
    std::string str(props_buffer.contents());
    str = "{" + str + "}";
    const auto msg = parseMessage(str);
    REQUIRE(msg.at("foo") == pj::value(42.0));
    REQUIRE(msg.at("bar") == pj::value(43.0));
  }

  SECTION("A child logger whose parent has logprops stores them for messages") {
    np::LogGroup rootlog(nullptr, nullptr, {{"foo", 42}});
    np::LogGroup childlog(&rootlog);

    REQUIRE(childlog.props() != nullptr);
    const auto& props_buffer = childlog.props()->data;
    std::string str(props_buffer.contents());
    str = "{" + str + "}";
    const auto msg = parseMessage(str);
    REQUIRE(msg.at("foo") == pj::value(42.0));
  }

  SECTION("A child logger whose ancestor has logprops stores them for messages") {
    np::LogGroup rootlog(nullptr, nullptr, {{"foo", 42}});
    np::LogGroup parentlog(&rootlog);
    np::LogGroup childlog(&parentlog);

    const auto& props_buffer = childlog.props()->data;
    std::string str(props_buffer.contents());
    str = "{" + str + "}";
    const auto msg = parseMessage(str);
    REQUIRE(msg.at("foo") == pj::value(42.0));
  }

  SECTION("A child logger with logprops should merge them with those from ancestors") {
    np::LogGroup rootlog(nullptr, nullptr, {{"foo", 42}});
    np::LogGroup childlog(&rootlog, nullptr, {{"bar", std::string("value")}});

    const auto& props_buffer = childlog.props()->data;
    std::string str(props_buffer.contents());
    str = "{" + str + "}";
    const auto msg = parseMessage(str);
    REQUIRE(msg.at("foo") == pj::value(42.0));
    REQUIRE(msg.at("bar") == pj::value("value"));
  }

  SECTION("Multiple logprops are merged correctly") {
    np::LogGroup rootlog(nullptr, nullptr, {{"a", 42}, {"b", 43}});
    np::LogGroup childlog(
      &rootlog, nullptr, {{"c", std::string("value0")}, {"d", std::string("value1")}});

    const auto& props_buffer = childlog.props()->data;
    std::string str(props_buffer.contents());
    str = "{" + str + "}";
    const auto msg = parseMessage(str);
    REQUIRE(msg.at("a") == pj::value(42.0));
    REQUIRE(msg.at("b") == pj::value(43.0));
    REQUIRE(msg.at("c") == pj::value("value0"));
    REQUIRE(msg.at("d") == pj::value("value1"));
  }

  SECTION("Child logger logprops take precedence") {
    np::LogGroup rootlog(nullptr, nullptr, {{"foo", 42}});
    np::LogGroup childlog(&rootlog, nullptr, {{"foo", std::string("value")}});

    const auto& props_buffer = childlog.props()->data;
    std::string str(props_buffer.contents());
    str = "{" + str + "}";
    const auto msg = parseMessage(str);
    REQUIRE(msg.at("foo") == pj::value("value"));
  }
}

TEST_CASE("Messages include the loggers logprops") {
  std::string out;
  np::log::Config cfg;
  cfg.fields = static_cast<np::log::Fields>(0);
  cfg.sink = [&](const np::log::MessageInfo& mi) { out = mi.message; };
  np::log::applyConfig(cfg);
  np::LogGroup log(nullptr, nullptr, {{"foo", 42}});
  { np::log::ScopedMessage msg(log, "", 0, -1, 0, ""); }
  auto msg = parseMessage(out);
  REQUIRE(msg.at("group").get<pj::object>().at("foo") == pj::value(42.0));
}
