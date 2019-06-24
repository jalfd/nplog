#include <nplog/ScopedMessage.hpp>
#include <catch/catch.hpp>

TEST_CASE("Log") {
  SECTION("Buffers are acquired and reused") {
    np::Log log;
    auto buf = log.acquireBuffer();
    buf.reserve(20);
    log.releaseBuffer(std::move(buf));
    auto b2 = log.acquireBuffer();
    CHECK(b2.capacity() == 20);
  }

  SECTION("Buffers are cleared on reuse") {
    np::Log log;
    auto buf = log.acquireBuffer();
    buf.push_back('x');
    log.releaseBuffer(std::move(buf));
    auto b2 = log.acquireBuffer();
    CHECK(b2.empty());
  }
}
