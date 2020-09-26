#include <nplog/messagebuffer.hpp>
#include <nplog/scopedmessage.hpp>
#include <picojson/picojson.h>
#include <vector>
#include <catch/catch.hpp>
// FIXME: also needs some consideration for testing derived class bringing things together -
// including that it imports static params, and consider buffer allocation

namespace pj = picojson;

static pj::object parseLogMessage(const np::log::MessageBuffer& buf) {
  pj::value val;
  std::string err;
  const auto contents = buf.contents();
  picojson::parse(val, contents.begin(), contents.end(), &err);
  CAPTURE(contents);
  REQUIRE(err == "");
  REQUIRE(val.is<pj::object>());
  return val.get<pj::object>();
}

namespace testns {
  struct CustomParamType {};
} // namespace testns

namespace np::log {
  template <>
  struct Formatter<testns::CustomParamType> {
    void operator()(const testns::CustomParamType& val, ValueSerializer& srl) {
      srl.write("a custom value");
    }
  };
} // namespace np::log

bool operator==(const std::vector<char>& result, const std::string& expected) {
  std::vector<char> e(expected.begin(), expected.end());
  return result == e;
}

TEST_CASE("ScopedMessageBase") {
  np::log::Config cfg;
  cfg.fields = static_cast<np::log::Config::Fields>(0);
  np::log::applyConfig(cfg);
  SECTION("ScopedMessage writes message to the buffer") {
    np::log::ScopedMessageBase msg(
      "file", 3, 1, "hello", np::log::MessageBuffer(), "name", {});
    msg.endMessage();
    const auto& buffer = msg.buffer();
    const auto obj = parseLogMessage(buffer);

    CHECK(obj.size() == 1);
    CHECK(obj.at("message") == pj::value("hello"));
  }

  SECTION("ScopedMessage writes header fields to the buffer") {
    cfg.fields = static_cast<np::log::Config::Fields>(-1);
    np::log::applyConfig(cfg);

    np::log::ScopedMessageBase msg(
      "file", 3, 1, "hello", np::log::MessageBuffer(), "name", {});
    msg.endMessage();
    const auto& buffer = msg.buffer();
    const auto obj = parseLogMessage(buffer);

    CHECK(obj.at("file") == pj::value("file"));
    CHECK(obj.at("level") == pj::value(1.0));
    CHECK(obj.at("message") == pj::value("hello"));
    CHECK(obj.at("log") == pj::value("name"));
  }

  SECTION("ScopedMessage writes parameters with standard types to the buffer") {
    np::log::ScopedMessageBase msg("", 0, 0, "", np::log::MessageBuffer(), "", {});
    msg.addParam("number", 42);
    msg.addParam("string", std::string_view("42"));
    msg.endMessage();
    const auto& buffer = msg.buffer();
    const auto obj = parseLogMessage(buffer);

    CHECK(obj.at("params").is<pj::object>());
    const auto params = obj.at("params").get<pj::object>();
    CHECK(params.size() == 2);
    CHECK(params.at("number") == pj::value(42.0));
    CHECK(params.at("string") == pj::value("42"));
  }

  SECTION("ScopedMessage writes parameters with custom types to the buffer") {
    testns::CustomParamType p;
    np::log::ScopedMessageBase msg("", 0, 0, "", np::log::MessageBuffer(), "", {});
    msg.addParam("p", p);
    msg.endMessage();
    const auto& buffer = msg.buffer();
    const auto obj = parseLogMessage(buffer);

    CHECK(obj.at("params").is<pj::object>());
    const auto params = obj.at("params").get<pj::object>();
    CHECK(params.size() == 1);
    CHECK(params.at("p") == pj::value("a custom value"));
  }
}
TEST_CASE("ScopedMessage") {
  SECTION("ScopedMessage can handle reentrancy") { // TODO: needs to be rewritten. Must work on SM,
                                                   // not SMBase level
    pj::object inner_message;
    const auto nested = [&]() {
      np::log::ScopedMessageBase msg("", 0, 0, "", np::log::MessageBuffer(), "", {});
      msg.addParam("name", std::string_view("inner"));
      msg.endMessage();
      inner_message = parseLogMessage(msg.buffer());
      return std::string("outer");
    };

    np::log::ScopedMessageBase msg("", 0, 0, "", np::log::MessageBuffer(), "", {});
    msg.endMessage();
    msg.addParam("name", nested());
  }
}
