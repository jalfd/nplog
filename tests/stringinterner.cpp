#include "../src/stringinterner.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace np::log;

TEST_CASE("StringInterner") {
  GIVEN("a string interner") {
    StringInterner<16> si;

    WHEN("a string is interned") {
      const char* s0 = "hello";
      const auto sv0 = si.intern(s0);
      THEN("the string is JSON-formatted") { CHECK(sv0 == "\"hello\""); }

      THEN("subsequent requests with the same char* return the same string") {
        CHECK(si.intern(s0).data() == sv0.data());
      }

      THEN(
        "subsequent requests with different pointers to identical strings return the same string") {
        // this is unsafe, as input strings should never go out of scope, but for testing with a
        // temporary interner we can get away with it if we're careful
        const std::string s1 = "hello";
        CAPTURE((void*) si.intern(s1.c_str()).data());
        CAPTURE((void*) sv0.data());
        CAPTURE(si.intern(s1.c_str()).size());
        CHECK(si.intern(s1.c_str()).data() == sv0.data());
      }

      AND_WHEN("a second string is interned, exactly filling up the current buffer") {
        const char* s1 = "world";
        const auto sv1 = si.intern(s1);
        THEN("the string is interned contiguously with the previous string") {
          CHECK(sv0.data() + 8 == sv1.data());
        }
      }

      AND_WHEN("a second string is interned, overflowing the current buffer") {
        const char* s1 = "woorld";
        const auto sv1 = si.intern(s1);
        THEN("the string is interned non-contiguously with the previous string") {
          CHECK(sv0.data() + 8 != sv1.data());
        }
      }
    }

    WHEN("a string is interned that is bigger than the default buffer size") {
      const char* s1 = "hello world!";
      const auto sv1 = si.intern(s1);

      THEN("the string is interned correctly") { CHECK(si.intern(s1) == sv1.data()); }
    }
  }
}