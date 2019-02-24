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

  SECTION("Submitting a buffer sends it to the sink function") {
    np::Log log;
    const char* msg_start = nullptr;
    size_t msg_len = 0;
    np::Log::setSink([&](std::string_view msg) {
      msg_start = &msg[0];
      msg_len = msg.size();
    });

    auto buf = log.acquireBuffer();
    buf.push_back('x');
    log.submitMessage(buf);
    CHECK(msg_start == &buf[0]);
    CHECK(msg_len == 1);
  }
}
