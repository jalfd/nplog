#include <nplog/config.hpp>
#include <nplog/loggroup.hpp>
#include <nplog/macros.hpp>
#include <nplog/scopedmessage.hpp>
#include <picojson/picojson.h>
#include <optional>
#include <catch/catch.hpp>

namespace pj = picojson;
using namespace np::log;

namespace {
  template <int I>
  struct Chatty {
    static inline int ctor = 0;
    static inline int dtor = 0;
    static inline int copyctor = 0;
    static inline int movector = 0;
    static inline int copyassign = 0;
    static inline int moveassign = 0;

    static void dump() {
      std::cout << "constructed: " << ctor << '\n'
                << "destroyed: " << dtor << '\n'
                << "copy constructed: " << copyctor << '\n'
                << "move constructed: " << movector << '\n'
                << "copy assigned: " << copyassign << '\n'
                << "move assigned: " << moveassign << std::endl;
    }

    Chatty() { ++ctor; }
    ~Chatty() { ++dtor; }
    Chatty(const Chatty&) { ++copyctor; }
    Chatty(Chatty&&) { ++movector; }
    Chatty& operator=(const Chatty&) { ++copyassign; }
    Chatty& operator=(Chatty&&) { ++moveassign; }
  };

  int sum(int i, int j) { return i + j; }
  std::string concat(int i, int j) { return std::to_string(i) + std::to_string(j); }
  int num_calls = 0;
  int countCalls() { return ++num_calls; }
} // namespace

template <int N>
struct np::log::Formatter<Chatty<N>> {
  void operator()(const Chatty<N>&, ValueSerializer& vs) { vs.write(0); }
};

TEST_CASE("macros") {
  np::log::Config cfg;
  cfg.levels.default_level
    = {threshold(Status), threshold(Status)};
  std::optional<level_type> logged_level;
  std::optional<pj::object> logged_message;
  cfg.sink = [&](MessageInfo mi) {
    REQUIRE(!logged_level.has_value());
    REQUIRE(!logged_message.has_value());
    pj::value val;
    std::string err;
    pj::parse(val, mi.message.begin(), mi.message.end(), &err);
    REQUIRE(err.empty());
    logged_message = val.get<pj::object>();
    logged_level = mi.level;
  };
  applyConfig(cfg);

  LogGroup logger;

  num_calls = 0;

  SECTION("Log a message with no props") {
    LOG(logger, Status, "this is a message");

    REQUIRE(*logged_level == Status);
    REQUIRE(logged_message->at("message") == pj::value("this is a message"));
  }

  SECTION("Log a message with a prop with implicit level and name") {
    LOG(logger, Status, "this is a message", NP_WITH(sum(1, 2)));

    REQUIRE(*logged_level == Status);
    const auto props = logged_message->at("props").get<pj::object>();
    REQUIRE(props.at("sum(1, 2)") == pj::value(3.0));
  }

  SECTION("Log a message with a prop with implicit level and explict name") {
    LOG(logger, Status, "this is a message", NP_WITH("sum of 1 and 2", sum(1, 2)));

    REQUIRE(*logged_level == Status);
    const auto props = logged_message->at("props").get<pj::object>();
    REQUIRE(props.at("sum of 1 and 2") == pj::value(3.0));
  }

  SECTION("Log a message with a prop with explicit level and implicit name") {
    LOG(logger, Status, "this is a message", NP_WITH(Error, sum(1, 2)));

    REQUIRE(*logged_level == Status);
    const auto props = logged_message->at("props").get<pj::object>();
    REQUIRE(props.at("sum(1, 2)") == pj::value(3.0));
  }

  SECTION("Log a message with a prop with explicit level and name") {
    LOG(logger,
      Status,
      "this is a message",
      NP_WITH(Error, "sum of 1 and 2", sum(1, 2)));

    REQUIRE(*logged_level == Status);
    const auto props = logged_message->at("props").get<pj::object>();
    REQUIRE(props.at("sum of 1 and 2") == pj::value(3.0));
  }

  SECTION("Log a message with multiple props") {
    LOG(logger, Status, "this is a message", NP_WITH(sum(1, 2)), NP_WITH(concat(1, 2)));

    REQUIRE(*logged_level == Status);
    const auto props = logged_message->at("props").get<pj::object>();
    REQUIRE(props.at("sum(1, 2)") == pj::value(3.0));
    REQUIRE(props.at("concat(1, 2)") == pj::value("12"));
  }

  SECTION("Don't evaluate a message if its level causes it to be skipped") {
    LOG(logger, DebugHigh, "this is a message", NP_WITH(countCalls()));

    REQUIRE(!logged_level.has_value());
    REQUIRE(!logged_message.has_value());
    REQUIRE(num_calls == 0);
  }

  SECTION("Don't evaluate a prop if its level causes it to be skipped") {
    LOG(logger, Status, "this is a message", NP_WITH(DebugHigh, countCalls()));

    REQUIRE(logged_level.has_value());
    REQUIRE(logged_message.has_value());
    REQUIRE(logged_message->find("props") == logged_message->end());
    REQUIRE(num_calls == 0);
  }

  SECTION("Props are never copied or moved") {
    LOG(logger, Status, "this is a message", NP_WITH(Chatty<0>()));
    CHECK(Chatty<0>::ctor == 1);
    CHECK(Chatty<0>::dtor == 1);
    CHECK(Chatty<0>::copyctor == 0);
    CHECK(Chatty<0>::movector == 0);
    CHECK(Chatty<0>::copyassign == 0);
    CHECK(Chatty<0>::moveassign == 0);
  }
}
