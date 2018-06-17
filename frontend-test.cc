#include <utility>
#include "frontend.hpp"
#include <catch.hpp>

using np::log::internal::literal;

namespace {
  template <typename T>
  using FrontendArg = std::tuple<np::log::Severity, std::string_view, const T*>;
  template <typename T>
  using BackendArg = std::tuple<bool, std::string_view, const T*>;

  const np::log::Header hdr{np::log::internal::literal(""),
    np::log::internal::literal(""),
    np::log::Severity::Error};

  template <typename... Args>
  struct TestData {
    np::log::Header h;
    std::string_view msg;
    std::tuple<BackendArg<Args>...> args;
    int call_count = 0;
  };
  template <typename... Args>
  struct TestComposer {
    void write(np::log::Header h, std::string_view msg, BackendArg<Args>... args) {
      ptr->h = h;
      ptr->msg = msg;
      ptr->args = std::tuple<BackendArg<Args>...>(args...);
      ++ptr->call_count;
    }

    TestData<Args...>* ptr;
  };
} // namespace

TEST_CASE("frontend header") {
  TestComposer<> c;
  std::remove_reference_t<decltype(*(c.ptr))> data;
  c.ptr = &data;

  np::log::Log<decltype(c)> log(c);

  np::log::scopedMessage(log, {literal("file"), literal("42"), np::log::Severity::Info})("");
  REQUIRE(data.call_count == 1);
  CHECK(data.h.file == "file");
  CHECK(data.h.line == "42");
  CHECK(data.h.severity == np::log::Severity::Info);
}

TEST_CASE("Frontend no args") {
  TestComposer<> c;
  std::remove_reference_t<decltype(*(c.ptr))> data;
  c.ptr = &data;

  np::log::Log<decltype(c)> log(c);

  np::log::ScopedMessage(log, hdr)("message");
  REQUIRE(data.call_count == 1);
  CHECK(data.msg == std::string_view("message"));
  CHECK(std::tuple_size<decltype(data.args)>::value == 0);
}

TEST_CASE("frontend 1 arg") {
  TestComposer<UDT> c;
  std::remove_reference_t<decltype(*(c.ptr))> data;
  c.ptr = &data;

  np::log::Log<decltype(c)> log(c);

  UDT udt;
  np::log::scopedMessage(log, hdr)(
    "", FrontendArg<UDT>{np::log::Severity::All, std::string_view("argname"), &udt});
  REQUIRE(data.call_count == 1);
  CHECK(std::tuple_size<decltype(data.args)>::value == 1);
  const auto& arg0 = std::get<0>(data.args);
  CHECK(std::get<0>(arg0) == true);
  CHECK(std::get<1>(arg0) == "argname");
  CHECK(std::get<2>(arg0) == &udt);
}

TEST_CASE("frontend 2 args") {
  TestComposer<UDT, bool> c;
  std::remove_reference_t<decltype(*(c.ptr))> data;
  c.ptr = &data;

  np::log::Log<decltype(c)> log(c);

  UDT udt;
  bool b;
  np::log::scopedMessage(log, hdr)("",
    FrontendArg<UDT>{np::log::Severity::All, std::string_view("arg0"), &udt},
    FrontendArg<bool>{np::log::Severity::All, std::string_view("arg1"), &b});
  REQUIRE(data.call_count == 1);
  CHECK(std::tuple_size<decltype(data.args)>::value == 2);
  const auto& arg0 = std::get<0>(data.args);
  CHECK(std::get<0>(arg0) == true);
  CHECK(std::get<1>(arg0) == "arg0");
  CHECK(std::get<2>(arg0) == &udt);

  const auto& arg1 = std::get<1>(data.args);
  CHECK(std::get<0>(arg1) == true);
  CHECK(std::get<1>(arg1) == "arg1");
  CHECK(std::get<2>(arg1) == &b);
}

TEST_CASE("frontend 2 messages") {
  TestComposer<char> c;
  std::remove_reference_t<decltype(*(c.ptr))> data;
  c.ptr = &data;

  np::log::Log<decltype(c)> log(c);

  char c0;
  char c1;
  np::log::scopedMessage(log, hdr)(
    "msg0", FrontendArg<char>{np::log::Severity::All, std::string_view("arg0"), &c0});
  REQUIRE(data.call_count == 1);
  CHECK(std::tuple_size<decltype(data.args)>::value == 1);
  const auto& arg = std::get<0>(data.args);
  CHECK(data.msg == std::string_view("msg0"));
  CHECK(std::get<0>(arg) == true);
  CHECK(std::get<1>(arg) == "arg0");
  CHECK(std::get<2>(arg) == &c0);

  np::log::scopedMessage(log, hdr)(
    "msg1", FrontendArg<char>{np::log::Severity::All, std::string_view("arg1"), &c1});

  REQUIRE(data.call_count == 2);
  CHECK(std::tuple_size<decltype(data.args)>::value == 1);
  CHECK(data.msg == std::string_view("msg1"));
  CHECK(std::get<0>(arg) == true);
  CHECK(std::get<1>(arg) == "arg1");
  CHECK(std::get<2>(arg) == &c1);
}

SCENARIO("frontend filter message severity") {
  GIVEN("a logger") {
    TestComposer<char> c;
    std::remove_reference_t<decltype(*(c.ptr))> data;
    c.ptr = &data;
    char arg;

    np::log::Log<decltype(c)> log(c);

    WHEN("log level is high") {
      log.setSeverity(np::log::Severity::Debug);

      THEN("messages with lower levels are not logged") {
          np::log::Header h = hdr;
          h.severity = np::log::Severity::Trace;
        np::log::scopedMessage(log, h)(
          "msg0", FrontendArg<char>{np::log::Severity::Trace, std::string_view("arg0"), &arg});
        CHECK(data.call_count == 0);
      }
      THEN("messages with higher levels are logged") {
          np::log::Header h = hdr;
          h.severity = np::log::Severity::Info;
        np::log::scopedMessage(log, h)(
          "msg0", FrontendArg<char>{np::log::Severity::Info, std::string_view("arg0"), &arg});
        CHECK(data.call_count == 1);
      }
      THEN("messages with equal levels are logged") {
          np::log::Header h = hdr;
          h.severity = np::log::Severity::Debug;
        np::log::scopedMessage(log, h)(
          "msg0", FrontendArg<char>{np::log::Severity::Debug, std::string_view("arg0"), &arg});
        CHECK(data.call_count == 1);
      }
    }
    WHEN("log level is low") {
      log.setSeverity(np::log::Severity::Warning);

      THEN("messages with lower levels are not logged") {
          np::log::Header h = hdr;
          h.severity = np::log::Severity::Info;
        np::log::scopedMessage(log, h)(
          "msg0", FrontendArg<char>{np::log::Severity::Info, std::string_view("arg0"), &arg});
        CHECK(data.call_count == 0);
      }
      THEN("messages with higher levels are logged") {
          np::log::Header h = hdr;
          h.severity = np::log::Severity::Error;
        np::log::scopedMessage(log, h)(
          "msg0", FrontendArg<char>{np::log::Severity::Error, std::string_view("arg0"), &arg});
        CHECK(data.call_count == 1);
      }
      THEN("messages with equal levels are logged") {
          np::log::Header h = hdr;
          h.severity = np::log::Severity::Warning;
        np::log::scopedMessage(log, h)(
          "msg0", FrontendArg<char>{np::log::Severity::Warning, std::string_view("arg0"), &arg});
        CHECK(data.call_count == 1);
      }
    }
  }
}

TEST_CASE("frontend filter argument severity") {
  GIVEN("a logger") {
    TestComposer<char> c;
    std::remove_reference_t<decltype(*(c.ptr))> data;
    c.ptr = &data;
    char arg;

    np::log::Log<decltype(c)> log(c);

    WHEN("arg level is higher than message level") {
      log.setSeverity(np::log::Severity::Debug, np::log::Severity::Warning);
      np::log::Header h = hdr;
      h.severity = np::log::Severity::Debug;

      THEN("args with lower levels are not logged") {
        np::log::scopedMessage(log, h)(
          "msg0", FrontendArg<char>{np::log::Severity::Info, std::string_view("arg0"), &arg});
        CHECK(data.call_count == 1);
        const auto &arg = std::get<0>(data.args);
        CHECK(!std::get<0>(arg));
      }
      THEN("args with higher levels are logged") {
          np::log::Header h = hdr;
          h.severity = np::log::Severity::Info;
        np::log::scopedMessage(log, h)(
          "msg0", FrontendArg<char>{np::log::Severity::Warning, std::string_view("arg0"), &arg});
        CHECK(data.call_count == 1);
        const auto &arg = std::get<0>(data.args);
        CHECK(std::get<0>(arg));
      }
      THEN("args with equal levels are logged") {
          np::log::Header h = hdr;
          h.severity = np::log::Severity::Debug;
        np::log::scopedMessage(log, h)(
          "msg0", FrontendArg<char>{np::log::Severity::Error, std::string_view("arg0"), &arg});
        CHECK(data.call_count == 1);
        const auto &arg = std::get<0>(data.args);
        CHECK(std::get<0>(arg));
      }
    }
  }
}
