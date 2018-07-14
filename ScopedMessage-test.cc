#include "ScopedMessage.hpp"
#include "test-utils.hpp"
#include <catch.hpp>

namespace np {
  namespace {
    struct Log {
      bool testMessage(int level) { return level < 5; }
    };
  } // namespace
} // namespace np

TEST_CASE("ScopedMessage") {
  SECTION("hi") {

  }
}
