#include <nplog/scopedmessage.hpp>
#include <catch/catch.hpp>

TEST_CASE("Log") {
  SECTION("Buffers are acquired and reused") {
    np::log::Log log;
    auto buf = log.acquireBuffer();
    buf.reserve(20);
    log.releaseBuffer(std::move(buf));
    auto b2 = log.acquireBuffer();
    CHECK(b2.bufferSize() == 20);
  }

  SECTION("Buffers are cleared on reuse") {
    np::log::Log log;
    auto buf = log.acquireBuffer();
    buf.append('x');
    log.releaseBuffer(std::move(buf));
    auto b2 = log.acquireBuffer();
    CHECK(b2.messageSize() == 0);
  }

  SECTION("Submitting a buffer sends it to the sink function") {
    np::log::Log log;
    np::log::level_type level = 0;
    const char* msg_start = nullptr;
    size_t msg_len = 0;
    np::log::Config cfg;
    cfg.levels.default_level = {9, 9};
    cfg.sink = [&](auto msg) {
      level = msg.level;
      msg_start = msg.message.data();
      msg_len = msg.message.size();
    };
    np::log::applyConfig(cfg);

    auto buf = log.acquireBuffer();
    buf.append('x');
    log.submitMessage(3, buf);
    CHECK(msg_start == buf.contents().data());
    CHECK(msg_len == 1);
    CHECK(level == 3);
  }
}
