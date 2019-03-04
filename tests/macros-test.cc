#include <nplog/macros.hpp>
#include "test-utils.hpp"
#include <nplog/Config.hpp>
#include <catch/catch.hpp>

namespace np {
  namespace {
    struct Log {
      Levels refreshLevels() { return {5, 5}; }
    };

    // Mock class for testing
    struct ScopedMessage {
      // mocked class interface
      ScopedMessage(Log&, const char*, int, int, const char* m, int) {
        msg = m;
        ++message_counter;
      }

      inline bool suppressParam(int level) { return level > level_threshold; }
      inline bool suppressParam(const char* = nullptr) { return suppressParam(default_level); }

      template <typename T>
      bool addArg(const char* name, const T& expr) {
        serialize_callback(name, &expr);
        return true;
      }

      // for testability only
      static inline std::string msg;
      static inline int level_threshold = 5;
      static inline int default_level = 3;
      static inline std::function<void(const char*, const void*)> serialize_callback;
      static inline int message_counter = 0;
    };
  } // namespace
} // namespace np

int foo(int i, int j) { return i + j; }

bool bar_called;
int bar() {
  bar_called = true;
  return 42;
}

TEST_CASE("macros") {
  int calls = 0;
  bar_called = false;
  np::ScopedMessage::serialize_callback = nullptr;
  np::ScopedMessage::level_threshold = 5;
  np::ScopedMessage::default_level = 3;
  np::ScopedMessage::message_counter = 0;

  SECTION("Low log level Messages are logged") {
    np::Log log;
    LOG(log, 0, "hello0");
    CHECK(np::ScopedMessage::msg == "hello0");
    CHECK(np::ScopedMessage::message_counter == 1);
  }

  SECTION("No args") {
    np::ScopedMessage::serialize_callback = [&](auto name, const void* expr) { ++calls; };

    np::Log log;
    LOG(log, 0, "hello1");
    CHECK(calls == 0);
    CHECK(np::ScopedMessage::msg == "hello1");
  }

  SECTION("Single arg with implicit name and level") {
    np::ScopedMessage::serialize_callback = [&](auto name, const void* expr) {
      ++calls;
      CHECK(name == "foo(1, 2)");
      CHECK(*static_cast<const int*>(expr) == 3);
    };
    np::Log log;
    LOG(log, 0, "hello2", ARG(foo(1, 2)));
    CHECK(calls == 1);
    CHECK(np::ScopedMessage::msg == "hello2");
  }

  SECTION("Single arg with name and implicit level") {
    np::ScopedMessage::serialize_callback = [&](auto name, const void* expr) {
      ++calls;
      CHECK(name == "name");
      CHECK(*static_cast<const int*>(expr) == 3);
    };
    np::Log log;
    LOG(log, 0, "hello3", ARG("name", foo(1, 2)));
    CHECK(calls == 1);
    CHECK(np::ScopedMessage::msg == "hello3");
  }

  SECTION("Single arg with level and implicit name") {
    np::ScopedMessage::serialize_callback = [&](auto name, const void* expr) {
      ++calls;
      CHECK(name == "foo(1, 2)");
      CHECK(*static_cast<const int*>(expr) == 3);
    };
    np::Log log;
    LOG(log, 0, "hello4", ARG(2, foo(1, 2)));
    CHECK(calls == 1);
    CHECK(np::ScopedMessage::msg == "hello4");
  }

  SECTION("Single explicit arg") {
    np::ScopedMessage::serialize_callback = [&](auto name, const void* expr) {
      ++calls;
      CHECK(name == "name");
      CHECK(*static_cast<const int*>(expr) == 42);
    };
    np::Log log;
    LOG(log, 0, "hello5", ARG(2, "name", bar()));
    CHECK(calls == 1);
    CHECK(np::ScopedMessage::msg == "hello5");
    CHECK(bar_called);
  }

  SECTION("Multiple args") {
    np::ScopedMessage::serialize_callback = [&](auto name, const void* expr) {
      if (++calls == 1) {
        CHECK(name == "2 + 2");
        CHECK(*static_cast<const int*>(expr) == 4);
      } else {
        CHECK(name == "3 + 3");
        CHECK(*static_cast<const int*>(expr) == 6);
      }
    };
    np::Log log;
    LOG(log, 0, "hello6", ARG(2 + 2), ARG(3 + 3));
    CHECK(calls == 2);
    CHECK(np::ScopedMessage::msg == "hello6");
  }

  SECTION("Discard param if explicit log level is too high") {
    np::ScopedMessage::serialize_callback = [&](auto name, const void* expr) {
      ++calls;
      CHECK(name == "'x'");
      CHECK(*static_cast<const char*>(expr) == 'x');
    };
    np::Log log;
    LOG(log, 0, "hello7", ARG(5, 'x'), ARG(6, bar()));
    CHECK(calls == 1);
    CHECK(np::ScopedMessage::msg == "hello7");
    CHECK(!bar_called);
  }

  SECTION("Discard param if implicit log level is too high") {
    np::ScopedMessage::serialize_callback = [&](auto name, const void* expr) {
      ++calls;
    };

    np::ScopedMessage::default_level = 6;
    np::Log log;
    LOG(log, 0, "hello8", ARG(bar()));
    CHECK(calls == 0);
    CHECK(np::ScopedMessage::msg == "hello8");
    CHECK(!bar_called);
  }

  SECTION("Argument type is not copied or moved") {
    np::ScopedMessage::serialize_callback = [&](auto name, const void* expr) { ++calls; };

    np::Log log;
    LOG(log, 0, "hello9", ARG(Chatty<0>{}));
    CHECK(calls == 1);
    CHECK(Chatty<0>::ctor == 1);
    CHECK(Chatty<0>::dtor == 1);
    CHECK(Chatty<0>::copyctor == 0);
    CHECK(Chatty<0>::movector == 0);
    CHECK(Chatty<0>::copyassign == 0);
    CHECK(Chatty<0>::moveassign == 0);
  }

  SECTION("Don't evaluate message if level is too low") {
      np::Log log;
      LOG(log, 9, "hello10");
      CHECK(np::ScopedMessage::message_counter == 0);
  }

  SECTION("Don't evaluate argument if message is discarded") {
    np::ScopedMessage::serialize_callback = [&](auto name, const void* expr) {
      ++calls;
    };

    np::ScopedMessage::default_level = 6;
    np::Log log;
    LOG(log, 9, "hello11", ARG('x'));
    CHECK(calls == 0);
  }
}
