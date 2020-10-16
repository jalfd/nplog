#include <nplog/messagebuffer.hpp>
#include <catch/catch.hpp>

TEST_CASE("MessageBuffer") {
  np::log::MessageBuffer buf;

  static_assert(!std::is_copy_constructible_v<np::log::MessageBuffer>);
  static_assert(!std::is_copy_assignable_v<np::log::MessageBuffer>);

  SECTION("Constructor") {
    REQUIRE(buf.messageSize() == 0);
    REQUIRE(buf.bufferSize() == 0);
    REQUIRE(buf.contents().empty());
    REQUIRE(*buf.contents().begin() == '\0');
  }

  SECTION("Moving") {
    buf.append('x');
    SECTION("Move Constructor") {
      np::log::MessageBuffer buf2 = std::move(buf);
      REQUIRE(buf.bufferSize() == 0);
      REQUIRE(buf2.contents() == "x");
    }
    SECTION("Move Assignment") {
      np::log::MessageBuffer buf2;
      buf2 = std::move(buf);
      REQUIRE(buf.bufferSize() == 0);
      REQUIRE(buf2.contents() == "x");
    }
  }

  SECTION("Append char") {
    buf.append('x');
    REQUIRE(buf.messageSize() == 1);
    REQUIRE(buf.bufferSize() >= 1);
    REQUIRE(buf.contents() == "x");

    buf.append('y');
    REQUIRE(buf.messageSize() == 2);
    REQUIRE(buf.bufferSize() >= 2);
    REQUIRE(buf.contents() == "xy");
  }

  SECTION("Insert string") {
    REQUIRE(buf.bufferSize() == 0);
    char* pos = buf.insertAt(3);
    REQUIRE(buf.messageSize() == 3);
    REQUIRE(buf.bufferSize() >= 3);
    REQUIRE(buf.contents().size() == 3);
    std::fill_n(pos, 3, '-');

    pos = buf.insertAt(3);
    REQUIRE(buf.messageSize() == 6);
    REQUIRE(buf.bufferSize() >= 6);
    REQUIRE(buf.contents().size() == 6);
    std::fill_n(pos, 3, '_');

    REQUIRE(buf.contents() == "---___");
  }

  SECTION("Shrink") {
    auto pos = buf.insertAt(10);
    std::fill_n(pos, 10, '-');
    buf.shrinkTo(5);

    REQUIRE(buf.contents().size() == 5);
    REQUIRE(buf.contents().data()[5] == '\0');
  }

  SECTION("Clear") {
    auto pos = buf.insertAt(10);
    std::fill_n(pos, 10, '-');
    buf.clear();

    REQUIRE(buf.contents().empty());
  }
}
