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
    int level = 0;
    const char* msg_start = nullptr;
    size_t msg_len = 0;
    np::Log::setSink([&](int l, std::string_view msg) {
      level = l;
      msg_start = &msg[0];
      msg_len = msg.size();
    });

    auto buf = log.acquireBuffer();
    buf.push_back('x');
    log.submitMessage(3, buf);
    CHECK(msg_start == &buf[0]);
    CHECK(msg_len == 1);
    CHECK(level == 3);
  }
}
