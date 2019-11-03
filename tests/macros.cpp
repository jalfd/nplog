#include <nplog/config.hpp>
#include <nplog/macros.hpp>
#include "utils.hpp"
#include <catch/catch.hpp>

namespace np::log {
  namespace {
    struct Logger {
      LevelSpec refreshLevels(unsigned) { return {5, 5}; }
      unsigned knownVersion() { return 0; }
    };

    // Mock class for testing
    struct ScopedMessage {
      ScopedMessage(Logger&, const char*, int, int, const char* m, int) {
        msg = m;
        ++message_counter;
      }

      inline bool suppressParam(int level) { return level > level_threshold; }
      inline bool suppressParam(const char* = nullptr) { return suppressParam(default_level); }

      template <typename T>
      bool addParam(const char* name, const T& expr) {
        serialize_callback(name, &expr);
        return true;
      }

      // for testability only
      static inline std::string msg;
      static inline int level_threshold = 5;
      static inline int default_level = 3;
      static inline std::function<void(std::string, const void*)> serialize_callback;
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
  np::log::ScopedMessage::serialize_callback = nullptr;
  np::log::ScopedMessage::level_threshold = 5;
  np::log::ScopedMessage::default_level = 3;
  np::log::ScopedMessage::message_counter = 0;

  SECTION("Low log level Messages are logged") {
    np::log::Logger log;
    NP_LOG(log, 0, "hello0");
    CHECK(np::log::ScopedMessage::msg == "hello0");
    CHECK(np::log::ScopedMessage::message_counter == 1);
  }

  SECTION("No params") {
    np::log::ScopedMessage::serialize_callback = [&](auto name, const void* expr) { ++calls; };

    np::log::Logger log;
    NP_LOG(log, 0, "hello1");
    CHECK(calls == 0);
    CHECK(np::log::ScopedMessage::msg == "hello1");
  }

  SECTION("Single param with implicit name and level") {
    np::log::ScopedMessage::serialize_callback = [&](auto name, const void* expr) {
      ++calls;
      CHECK(name == "foo(1, 2)");
      CHECK(*static_cast<const int*>(expr) == 3);
    };
    np::log::Logger log;
    NP_LOG(log, 0, "hello2", NP_WITH(foo(1, 2)));
    CHECK(calls == 1);
    CHECK(np::log::ScopedMessage::msg == "hello2");
  }

  SECTION("Single param with name and implicit level") {
    np::log::ScopedMessage::serialize_callback = [&](auto name, const void* expr) {
      ++calls;
      CHECK(name == "name");
      CHECK(*static_cast<const int*>(expr) == 3);
    };
    np::log::Logger log;
    NP_LOG(log, 0, "hello3", NP_WITH("name", foo(1, 2)));
    CHECK(calls == 1);
    CHECK(np::log::ScopedMessage::msg == "hello3");
  }

  SECTION("Single param with level and implicit name") {
    np::log::ScopedMessage::serialize_callback = [&](auto name, const void* expr) {
      ++calls;
      CHECK(name == "foo(1, 2)");
      CHECK(*static_cast<const int*>(expr) == 3);
    };
    np::log::Logger log;
    NP_LOG(log, 0, "hello4", NP_WITH(2, foo(1, 2)));
    CHECK(calls == 1);
    CHECK(np::log::ScopedMessage::msg == "hello4");
  }

  SECTION("Single explicit param") {
    np::log::ScopedMessage::serialize_callback = [&](auto name, const void* expr) {
      ++calls;
      CHECK(name == "name");
      CHECK(*static_cast<const int*>(expr) == 42);
    };
    np::log::Logger log;
    NP_LOG(log, 0, "hello5", NP_WITH(2, "name", bar()));
    CHECK(calls == 1);
    CHECK(np::log::ScopedMessage::msg == "hello5");
    CHECK(bar_called);
  }

  SECTION("Multiple param") {
    np::log::ScopedMessage::serialize_callback = [&](auto name, const void* expr) {
      if (++calls == 1) {
        CHECK(name == "2 + 2");
        CHECK(*static_cast<const int*>(expr) == 4);
      } else {
        CHECK(name == "3 + 3");
        CHECK(*static_cast<const int*>(expr) == 6);
      }
    };
    np::log::Logger log;
    NP_LOG(log, 0, "hello6", NP_WITH(2 + 2), NP_WITH(3 + 3));
    CHECK(calls == 2);
    CHECK(np::log::ScopedMessage::msg == "hello6");
  }

  SECTION("Discard param if explicit log level is too high") {
    np::log::ScopedMessage::serialize_callback = [&](auto name, const void* expr) {
      ++calls;
      CHECK(name == "'x'");
      CHECK(*static_cast<const char*>(expr) == 'x');
    };
    np::log::Logger log;
    NP_LOG(log, 0, "hello7", NP_WITH(5, 'x'), NP_WITH(6, bar()));
    CHECK(calls == 1);
    CHECK(np::log::ScopedMessage::msg == "hello7");
    CHECK(!bar_called);
  }

  SECTION("Discard param if implicit log level is too high") {
    np::log::ScopedMessage::serialize_callback = [&](auto name, const void* expr) { ++calls; };

    np::log::ScopedMessage::default_level = 6;
    np::log::Logger log;
    NP_LOG(log, 0, "hello8", NP_WITH(bar()));
    CHECK(calls == 0);
    CHECK(np::log::ScopedMessage::msg == "hello8");
    CHECK(!bar_called);
  }

  SECTION("Parameters are not copied or moved") {
    np::log::ScopedMessage::serialize_callback = [&](auto name, const void* expr) { ++calls; };

    np::log::Logger log;
    NP_LOG(log, 0, "hello9", NP_WITH(Chatty<0>{}));
    CHECK(calls == 1);
    CHECK(Chatty<0>::ctor == 1);
    CHECK(Chatty<0>::dtor == 1);
    CHECK(Chatty<0>::copyctor == 0);
    CHECK(Chatty<0>::movector == 0);
    CHECK(Chatty<0>::copyassign == 0);
    CHECK(Chatty<0>::moveassign == 0);
  }

  SECTION("Don't evaluate message if level is too low") {
    np::log::Logger log;
    NP_LOG(log, 9, "hello10");
    CHECK(np::log::ScopedMessage::message_counter == 0);
  }

  SECTION("Don't evaluate parameter if message is discarded") {
    np::log::ScopedMessage::serialize_callback = [&](auto name, const void* expr) { ++calls; };

    np::log::ScopedMessage::default_level = 6;
    np::log::Logger log;
    NP_LOG(log, 9, "hello11", NP_WITH('x'));
    CHECK(calls == 0);
  }
}
