#include <nplog/logger.hpp>
#include <picojson/picojson.h>
#include <catch/catch.hpp>

namespace pj = picojson;

pj::object parseMessage(std::string_view contents) {
  pj::value val;
  std::string err;
  picojson::parse(val, contents.begin(), contents.end(), &err);
  CAPTURE(contents);
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

  SECTION("A logger with logparams should add them to messages") {
    np::Logger log(nullptr, nullptr, {{"foo", 42}});
    const auto& params_buffer = log.loggerParams()->data;
    std::string str(params_buffer.contents());
    str = "{" + str + "}";
    const auto msg = parseMessage(str);
    REQUIRE(msg.at("foo") == pj::value(42.0));
  }

  SECTION("A logger with multiple logparams should add them to messages") {
    np::Logger log(nullptr, nullptr, {{"foo", 42}, {"bar", 43}});
    const auto& params_buffer = log.loggerParams()->data;
    std::string str(params_buffer.contents());
    str = "{" + str + "}";
    const auto msg = parseMessage(str);
    REQUIRE(msg.at("foo") == pj::value(42.0));
    REQUIRE(msg.at("bar") == pj::value(43.0));
  }

  SECTION("A child logger whose parent has logparams should add them to messages") {
    np::Logger rootlog(nullptr, nullptr, {{"foo", 42}});
    np::Logger childlog(&rootlog);

    REQUIRE(childlog.loggerParams() != nullptr);
    const auto& params_buffer = childlog.loggerParams()->data;
    std::string str(params_buffer.contents());
    str = "{" + str + "}";
    const auto msg = parseMessage(str);
    REQUIRE(msg.at("foo") == pj::value(42.0));
  }

  SECTION("A child logger whose ancestor has logparams should add them to messages") {
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
    np::Logger childlog(&rootlog, nullptr, {{"c", std::string("value0")}, {"d", std::string("value1")}});
    np::Logger fakelog(nullptr, nullptr, {{"c", std::string("value0")}, {"d", std::string("value1")}});

    CAPTURE(rootlog.loggerParams()->data.contents());
    CAPTURE(fakelog.loggerParams()->data.contents());

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

TEST_CASE("woo") {
  SECTION("Multiple logparams are merged correctly") {
    np::Logger rootlog(nullptr, nullptr, {{"a", 42}, {"b", 43}});
    np::Logger childlog(&rootlog, nullptr, {{"c", std::string("value0")}, {"d", std::string("value1")}});
    np::Logger fakelog(nullptr, nullptr, {{"c", std::string("value0")}, {"d", std::string("value1")}});

auto *p = rootlog.loggerParams();
    CAPTURE(p->data.contents().substr(std::get<0>(p->offsets[0]), std::get<2>(p->offsets[0])));
    CAPTURE(p->data.contents().substr(std::get<0>(p->offsets[1]), std::get<2>(p->offsets[1])));
    CAPTURE(rootlog.loggerParams()->data.contents());
    CAPTURE(fakelog.loggerParams()->data.contents());

    const auto& params_buffer = childlog.loggerParams()->data;
    std::string str(params_buffer.contents());
    str = "{" + str + "}";
    const auto msg = parseMessage(str);
    REQUIRE(msg.at("a") == pj::value(42.0));
    REQUIRE(msg.at("b") == pj::value(43.0));
    REQUIRE(msg.at("c") == pj::value("value0"));
    REQUIRE(msg.at("d") == pj::value("value1"));
  }
}
