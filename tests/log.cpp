#include <nplog/messagebuffer.hpp>
#include <nplog/scopedmessage.hpp>
#include <catch/catch.hpp>

/*
TEST_CASE("Log") {
  SECTION("Submitting a buffer sends it to the sink function") {
    np::log::Logger log;
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

    auto buf = np::log::acquireBuffer();
    buf->append('x');
    log.submitMessage(3, *buf, np::log::currentVersion());
    CHECK(msg_start == buf->contents().data());
    np::log::releaseBuffer(std::move(buf));
    CHECK(msg_len == 1);
    CHECK(level == 3);
  }
}
*/
TEST_CASE("Log") { REQUIRE(false); }
