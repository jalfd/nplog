#include <nplog/ScopedMessage.hpp>
#include <catch/catch.hpp>

TEST_CASE("Log") {
  SECTION("Message level filtering") {
    np::Log log;
    REQUIRE(!log.suppressMessage(0));
    REQUIRE(log.suppressMessage(1));
    np::Log::setMessageLevel(3);
    REQUIRE(!log.suppressMessage(2));
    REQUIRE(!log.suppressMessage(3));
    REQUIRE(log.suppressMessage(4));
  }

  SECTION("Param level retrieval") {
    np::Log log;
    REQUIRE(log.paramLevel() == 0);
    np::Log::setParamLevel(3);
    REQUIRE(log.paramLevel() == 3);
  }

  SECTION("Buffers are acquired and reused") {
    np::Log log;
    auto buf = log.acquireBuffer();
    buf.reserve(20);
    log.releaseBuffer(std::move(buf));
    auto b2 = log.acquireBuffer();
    CHECK(b2.capacity() == 20);
  }
}
