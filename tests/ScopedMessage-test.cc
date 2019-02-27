#include <nplog/ScopedMessage.hpp>
#include <vector>
#include "mocks.hpp"
#include <catch/catch.hpp>

namespace test1 {
  struct Foo {};
} // namespace test1

namespace np {
  template <>
  struct Formatter<test1::Foo> {
    void operator()(const test1::Foo& val, ValueSerializer& srl) {
      ops.emplace_back("format", &val);
    }
  };
} // namespace np

namespace {
  struct MockLog {
    using buffer_type = MockBuffer;
    using serializer_type = MockSerializer;

    bool suppressMessage(int) const { return true; }
    int paramLevel() const { return 3; }

    buffer_type acquireBuffer() {
      ++buffersRequested;
      return buffer_type();
    }

    void submitMessage(buffer_type buffer) { ops.emplace_back("submitMessage", buffer.id); }
    void releaseBuffer(buffer_type buffer) { ops.emplace_back("releaseBuffer", buffer.id); }

    int buffersRequested = 0;
  };

} // namespace

bool operator==(const std::vector<char>& result, const std::string& expected) {
  std::vector<char> e(expected.begin(), expected.end());
  return result == e;
}

TEST_CASE("ScopedMessage") {
  ops.clear();
  MockLog log;

  SECTION("ScopedMessage requests a buffer") {
    np::ScopedMessage<MockLog> msg(log, "", 0, 0, "hello");
    CHECK(log.buffersRequested == 1);
  }

  SECTION("ScopedMessage writes its header to the buffer") {
    { np::ScopedMessage<MockLog> msg(log, "file", 3, 1, "hello"); }
    // this should ask for a buffer, and fill it appropriately
    CHECK(log.buffersRequested == 1);
    CHECK(ops.size() == 6);
    CHECK(std::get<0>(ops.at(0)) == "ctor");
    const auto buffer_id = std::any_cast<int>(std::get<1>(ops.at(0)));
    CHECK(std::get<0>(ops.at(1)) == "prologue");
    {
      using arg_type = std::tuple<std::string_view, int, int, std::string_view, int>;
      const auto [file, line, level, msg, bid] = std::any_cast<arg_type>(std::get<1>(ops.at(1)));
      CHECK(file == "file");
      CHECK(line == 3);
      CHECK(level == 1);
      CHECK(msg == "hello");
      CHECK(bid == buffer_id);
    }
    CHECK(std::get<0>(ops.at(2)) == "epilogue");
    CHECK(std::any_cast<int>(std::get<1>(ops.at(2))) == buffer_id);
    CHECK(std::get<0>(ops.at(3)) == "submitMessage");
    CHECK(std::any_cast<int>(std::get<1>(ops.at(3))) == buffer_id);
    CHECK(std::get<0>(ops.at(4)) == "releaseBuffer");
    CHECK(std::any_cast<int>(std::get<1>(ops.at(4))) == buffer_id);
    CHECK(std::get<0>(ops.at(5)) == "dtor");
  }

  SECTION("ScopedMessage writes arguments to the buffer") {
    test1::Foo foo;
    {
      np::ScopedMessage<MockLog> msg(log, "", 0, 0, "");
      msg.addArg("name", foo);
    }
    CHECK(log.buffersRequested == 1);
    CHECK(ops.size() == 8);
    CHECK(std::get<0>(ops.at(0)) == "ctor");
    CHECK(std::get<0>(ops.at(1)) == "prologue");
    CHECK(std::get<0>(ops.at(2)) == "writeKey");
    CHECK(std::any_cast<std::string_view>(std::get<1>(ops.at(2))) == "name");
    CHECK(std::get<0>(ops.at(3)) == "format");
    CHECK(std::any_cast<const test1::Foo*>(std::get<1>(ops.at(3))) == &foo);
    CHECK(std::get<0>(ops.at(4)) == "epilogue");
    CHECK(std::get<0>(ops.at(5)) == "submitMessage");
    CHECK(std::get<0>(ops.at(6)) == "releaseBuffer");
    CHECK(std::get<0>(ops.at(7)) == "dtor");
  }

  SECTION("Params are suppressed correctly") {
    np::ScopedMessage<MockLog> msg(log, "", 0, 0, "");
    CHECK(!msg.suppressParam(2));
    CHECK(!msg.suppressParam(3));
    CHECK(msg.suppressParam(4));
  }

  SECTION("ScopedMessage can handle reentrancy") {
    const auto nested = [&]() {
      np::ScopedMessage<MockLog>(log, "", 0, 0, "");
      return test1::Foo();
    };
    {
      np::ScopedMessage<MockLog> msg(log, "", 0, 0, "");
      msg.addArg("name", nested());
    }
    CHECK(log.buffersRequested == 2);
    CHECK(ops.size() == 14);
    // Outer message is created using buffer1 (buf not yet written)
    CHECK(std::get<0>(ops.at(0)) == "ctor");
    const auto buffer1 = std::any_cast<int>(std::get<1>(ops.at(0)));
    CHECK(std::get<0>(ops.at(1)) == "prologue");
    // Inner message is created using buffer2
    CHECK(std::get<0>(ops.at(2)) == "ctor");
    const auto buffer2 = std::any_cast<int>(std::get<1>(ops.at(2)));
    CHECK(std::get<0>(ops.at(3)) == "prologue");
    {
      using arg_type = std::tuple<std::string_view, int, int, std::string_view, int>;
      const auto bid = std::get<4>(std::any_cast<arg_type>(std::get<1>(ops.at(3))));
      CHECK(bid == buffer2);
    }
    CHECK(std::get<0>(ops.at(4)) == "epilogue");
    CHECK(std::any_cast<int>(std::get<1>(ops.at(4))) == buffer2);
    // Inner emessage is written
    CHECK(std::get<0>(ops.at(5)) == "submitMessage");
    CHECK(std::any_cast<int>(std::get<1>(ops.at(5))) == buffer2);
    CHECK(std::get<0>(ops.at(6)) == "releaseBuffer");
    CHECK(std::any_cast<int>(std::get<1>(ops.at(6))) == buffer2);
    CHECK(std::get<0>(ops.at(7)) == "dtor");
    CHECK(std::get<0>(ops.at(8)) == "writeKey");
    CHECK(std::get<0>(ops.at(9)) == "format");
    CHECK(std::get<0>(ops.at(10)) == "epilogue");
    CHECK(std::any_cast<int>(std::get<1>(ops.at(10))) == buffer1);
    // Outer message is written
    CHECK(std::get<0>(ops.at(11)) == "submitMessage");
    CHECK(std::any_cast<int>(std::get<1>(ops.at(11))) == buffer1);
    CHECK(std::get<0>(ops.at(12)) == "releaseBuffer");
    CHECK(std::any_cast<int>(std::get<1>(ops.at(12))) == buffer1);
    CHECK(std::get<0>(ops.at(13)) == "dtor");
  }
}
