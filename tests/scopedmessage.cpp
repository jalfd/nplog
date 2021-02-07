#include "../src/messagebuffer.hpp"
#include <nplog/config.hpp>
#include <nplog/scopedmessage.hpp>
#include <picojson/picojson.h>
#include <vector>
#include <catch/catch.hpp>
#include "../src/loggroupprops.hpp" // FIXME: better access from tests

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

static pj::object parseLogMessage(const np::log::MessageBuffer& buf) {
  return parseMessage(buf.contents());
}

namespace testns {
  struct CustomPropType {};
} // namespace testns

namespace np::log {
  template <>
  struct Formatter<testns::CustomPropType> {
    void operator()(const testns::CustomPropType& val, ValueSerializer& srl) {
      srl.write("a custom value");
    }
  };
} // namespace np::log

bool operator==(const std::vector<char>& result, const std::string& expected) {
  std::vector<char> e(expected.begin(), expected.end());
  return result == e;
}

TEST_CASE("ScopedMessageBase") {
  np::log::MessageBuffer buf;
  SECTION("ScopedMessage writes message to the buffer") {
    np::log::ScopedMessageBase msg("file", 3, static_cast<np::log::Fields>(0), 1, "hello", &buf, "name", {});
    msg.endMessage();
    const auto obj = parseLogMessage(buf);

    CHECK(obj.size() == 1);
    CHECK(obj.at("message") == pj::value("hello"));
  }

  SECTION("ScopedMessage writes header fields to the buffer") {
    np::log::ScopedMessageBase msg("file", 3, static_cast<np::log::Fields>(-1), 1, "hello", &buf, "name", {});
    msg.endMessage();
    const auto obj = parseLogMessage(buf);

    CHECK(obj.at("file") == pj::value("file"));
    CHECK(obj.at("level") == pj::value(1.0));
    CHECK(obj.at("message") == pj::value("hello"));
    CHECK(obj.at("log") == pj::value("name"));
  }

  SECTION("ScopedMessage writes properties with standard types to the buffer") {
    np::log::ScopedMessageBase msg("", 0, {}, 0, "", &buf, "", {});
    msg.addProp("number", 42);
    msg.addProp("string", std::string_view("42"));
    msg.endMessage();
    const auto obj = parseLogMessage(buf);

    CHECK(obj.at("props").is<pj::object>());
    const auto props = obj.at("props").get<pj::object>();
    CHECK(props.size() == 2);
    CHECK(props.at("number") == pj::value(42.0));
    CHECK(props.at("string") == pj::value("42"));
  }

  SECTION("ScopedMessage writes properties with custom types to the buffer") {
    testns::CustomPropType p;
    np::log::ScopedMessageBase msg("", 0, {}, 0, "", &buf,"", {});
    msg.addProp("p", p);
    msg.endMessage();
    const auto obj = parseLogMessage(buf);

    CHECK(obj.at("props").is<pj::object>());
    const auto props = obj.at("props").get<pj::object>();
    CHECK(props.size() == 1);
    CHECK(props.at("p") == pj::value("a custom value"));
  }
}
TEST_CASE("ScopedMessage") {
  SECTION("ScopedMessage can handle reentrancy") {
    std::vector<pj::object> out;

    np::log::Config cfg;
    cfg.fields = static_cast<np::log::Fields>(0);
    cfg.sink = [&](const np::log::MessageInfo& mi) { out.push_back(parseMessage(mi.message)); };
    np::log::applyConfig(cfg);

    np::log::LogGroup logger;
    {
      np::log::ScopedMessage msg(logger, "", 0, -1, 0, "outer message");
      msg.addProp("name", [&]() {
        np::log::ScopedMessage msg_inner(logger, "", 0, -1, 0, "inner message");
        msg_inner.addProp("name", std::string_view("inner"));
        return std::string("outer");
      }());
    }
    REQUIRE(out.size() == 2);
    REQUIRE(out[0].at("message") == pj::value("inner message"));
    REQUIRE(out[0].at("props").get<pj::object>().at("name") == pj::value("inner"));
    REQUIRE(out[1].at("message") == pj::value("outer message"));
    REQUIRE(out[1].at("props").get<pj::object>().at("name") == pj::value("outer"));
  }
}
