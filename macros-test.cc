#include <catch.hpp>
#include "macros.hpp"
#include "test-utils.hpp"

namespace {
  namespace np {
    // Mock class for testing
    struct ScopedMessage {
      ScopedMessage(const char* m) { msg = m; }

      inline bool testArg(int i) { return i < level_threshold; };
      inline bool testArg(const char* = nullptr) {
          return testArg(default_level);
      }

      template <typename T>
      bool serialize(const char* name, const T& expr) {
          serialize_callback(name, &expr);
          return true;
      }

      const char* getName(const char* name, const char*) { return name; }
      const char* getName(int, const char* name) { return name; }

      static inline std::string msg;
      static inline int level_threshold = 5;
      static inline int default_level = 3;
      static inline std::function<void(const char*, const void*)> serialize_callback;
    };
  } // namespace np
} // namespace

int foo(int i, int j) { return i + j; }

TEST_CASE("macros") {
  int calls = 0;
  np::ScopedMessage::serialize_callback = nullptr;
  np::ScopedMessage::level_threshold = 5;
  np::ScopedMessage::default_level = 3;

  SECTION("No args") {
    np::ScopedMessage::serialize_callback = [&](auto name, const void* expr) { ++calls; };

    LOG("hello1");
    CHECK(calls == 0);
    CHECK(np::ScopedMessage::msg == "hello1");
  }

  SECTION("Single arg with implicit name and level") {
    np::ScopedMessage::serialize_callback = [&](auto name, const void* expr) {
      ++calls;
      CHECK(name == "foo(1, 2)");
      CHECK(*static_cast<const int*>(expr) == 3);
    };
    LOG("hello2", ARG(foo(1, 2)));
    CHECK(calls == 1);
    CHECK(np::ScopedMessage::msg == "hello2");
  }

  SECTION("Single arg with name and implicit level") {
    np::ScopedMessage::serialize_callback = [&](auto name, const void* expr) {
      ++calls;
      CHECK(name == "name");
      CHECK(*static_cast<const int*>(expr) == 3);
    };
    LOG("hello3", ARG("name", foo(1, 2)));
    CHECK(calls == 1);
    CHECK(np::ScopedMessage::msg == "hello3");
  }

  SECTION("Single arg with level and implicit name") {
    np::ScopedMessage::serialize_callback = [&](auto name, const void* expr) {
      ++calls;
      CHECK(name == "foo(1, 2)");
      CHECK(*static_cast<const int*>(expr) == 3);
    };
    LOG("hello4", ARG(2, foo(1, 2)));
    CHECK(calls == 1);
    CHECK(np::ScopedMessage::msg == "hello4");
  }

  SECTION("Single explicit arg") {
    np::ScopedMessage::serialize_callback = [&](auto name, const void* expr) {
      ++calls;
      CHECK(name == "name");
      CHECK(*static_cast<const int*>(expr) == 3);
    };
    LOG("hello5", ARG(2, "name",foo(1, 2)));
    CHECK(calls == 1);
    CHECK(np::ScopedMessage::msg == "hello5");
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
    LOG("hello6", ARG(2 + 2), ARG(3 + 3));
    CHECK(calls == 2);
    CHECK(np::ScopedMessage::msg == "hello6");
  }

  SECTION("Discard if explicit log level is too high") {
    np::ScopedMessage::serialize_callback = [&](auto name, const void* expr) {
      ++calls;
      CHECK(name == "'x'");
      CHECK(*static_cast<const char*>(expr) == 'x');
    };
    LOG("hello7", ARG(4, 'x'), ARG(5, 'y'));
    CHECK(calls == 1);
    CHECK(np::ScopedMessage::msg == "hello7");
  }

  SECTION("Discard if implicit log level is too high") {
    np::ScopedMessage::serialize_callback = [&](auto name, const void* expr) {
      ++calls;
      CHECK(name == "'x'");
      CHECK(*static_cast<const char*>(expr) == 'x');
    };

    np::ScopedMessage::default_level = 5;
    LOG("hello8", ARG('x'));
    CHECK(calls == 0);
    CHECK(np::ScopedMessage::msg == "hello8");
  }

  SECTION("Argument type is not copied or moved") {
    np::ScopedMessage::serialize_callback = [&](auto name, const void* expr) {
      ++calls;
    };

    LOG("hello8", ARG(Chatty<0>{}));
    CHECK(calls == 1);
    CHECK(Chatty<0>::ctor == 1);
    CHECK(Chatty<0>::dtor == 1);
    CHECK(Chatty<0>::copyctor == 0);
    CHECK(Chatty<0>::movector == 0);
    CHECK(Chatty<0>::copyassign == 0);
    CHECK(Chatty<0>::moveassign == 0);
  }
}
