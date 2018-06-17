#include <memory>
#include "frontend.hpp"
#include <catch.hpp>

using np::log::internal::literal;

const np::log::Header hdr{np::log::internal::literal(""),
  np::log::internal::literal(""),
  np::log::Severity::Error};

TEST_CASE("frontend header") {
  struct TestData {
    np::log::Header h;
  };
  struct TestComposer {
    void write(np::log::Header h, std::string_view) { ptr->h = h; }

    TestData* ptr;
  };
  TestData data;
  np::log::Log<TestComposer> log({&data});
  np::log::scopedMessage(log, {literal("file"), literal("42"), np::log::Severity::Info})("");
  CHECK(data.h.file == "file");
  CHECK(data.h.line == "42");
  CHECK(data.h.severity == np::log::Severity::Info);
}

TEST_CASE("Frontend no args") {
  struct TestComposer {
    void write(np::log::Header h, std::string_view msg) {
      h2 = h;
      msg2 = msg;
    }

    np::log::Header h2;
    std::string_view msg2;
  };
  np::log::Log<TestComposer> log({});

  np::log::ScopedMessage msg(log, hdr);
}

TEST_CASE("frontend 1 arg") {
  struct TestData {
    np::log::Header h;
    std::string_view msg;
    std::tuple<bool, std::string_view, const UDT*> arg0;
    int call_count = 0;
  };
  struct TestComposer {
    void write(np::log::Header h,
      std::string_view msg,
      std::tuple<bool, std::string_view, const UDT*> arg0) {
      ptr->h = h;
      ptr->msg = msg;
      ptr->arg0 = arg0;
      ++ptr->call_count;
    }

    TestData* ptr;
  };
  const auto line = __LINE__ + 1;
  TestData data;
  np::log::Log<TestComposer> log({&data});
  UDT udt;
  np::log::scopedMessage(log, hdr)("hello world",
    std::tuple<np::log::Severity, std::string_view, const UDT*>{
      np::log::Severity::All, std::string_view("argname"), &udt});
  REQUIRE(data.call_count == 1);
  CHECK(data.msg == std::string_view("hello world"));
  CHECK(std::get<0>(data.arg0) == true);
  CHECK(std::get<1>(data.arg0) == "argname");
  CHECK(std::get<2>(data.arg0) == &udt);
}

