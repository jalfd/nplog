#include "macros.hpp"
#include "test-utils.hpp"
#include <catch.hpp>

// what happens in case of reentrancy?
// I guess it could just be a protocol where message ctor goes "can I have a buffer please?", and we
// either provide the one we have, or create a new one
// Also, need a reentrancy test then

namespace {
  namespace np {
    struct Log {
      bool testMessage(int level) { return level < 5; }

      // TODO: not needed in this test case, but implement where needed
      // caller should treat the buffer as an opaque type
      using buffer_type = std::vector<char>;
      // caller must be able to go "give me a buffer"
      buffer_type messageBuffer();
      // caller must be able to go "please serialize this for me and put it in this buffer"
      template <typename T>
      void serialize(const char* name, const T& expr, buffer_type& buffer);
      // caller must be able to go "ok, flush this message buffer (and take it back if you want it)
      void submitMessage(buffer_type buffer);
    };

    // Mock class for testing
    struct ScopedMessage {
      // mocked class interface
      ScopedMessage(const char* m) {
          msg = m;
          ++message_counter;
      }

      inline bool testArg(int i) { return i < level_threshold; };
      inline bool testArg(const char* = nullptr) { return testArg(default_level); }

      template <typename T>
      bool serialize(const char* name, const T& expr) {
        serialize_callback(name, &expr);
        return true;
      }

      // should be outside the class, so it doesn't need to be mocked
      const char* getName(const char* name, const char*) { return name; }
      const char* getName(int, const char* name) { return name; }

      // for testability only
      static inline std::string msg;
      static inline int level_threshold = 5;
      static inline int default_level = 3;
      static inline std::function<void(const char*, const void*)> serialize_callback;
      static inline int message_counter = 0;
    };
  } // namespace np
} // namespace

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

  SECTION("Discard if explicit log level is too high") {
    np::ScopedMessage::serialize_callback = [&](auto name, const void* expr) {
      ++calls;
      CHECK(name == "'x'");
      CHECK(*static_cast<const char*>(expr) == 'x');
    };
    np::Log log;
    LOG(log, 0, "hello7", ARG(4, 'x'), ARG(5, bar()));
    CHECK(calls == 1);
    CHECK(np::ScopedMessage::msg == "hello7");
    CHECK(!bar_called);
  }

  SECTION("Discard if implicit log level is too high") {
    np::ScopedMessage::serialize_callback = [&](auto name, const void* expr) {
      ++calls;
    };

    np::ScopedMessage::default_level = 5;
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

    np::ScopedMessage::default_level = 5;
    np::Log log;
    LOG(log, 9, "hello11", ARG('x'));
    CHECK(calls == 0);
  }
}
