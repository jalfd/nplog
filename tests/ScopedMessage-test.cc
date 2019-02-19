#include <vector>
#include <nplog/ScopedMessage.hpp>
#include <catch/catch.hpp>
#include "mocks.hpp"

namespace test1 {
    struct Foo {};
}

namespace np {
  template <>
  struct Formatter<test1::Foo>
  {
      void operator()(const test1::Foo& val, ValueSerializer& srl)
      {
          ops.emplace_back("format", &val);
      }
  };
}

namespace {
  struct MockLog {
    using buffer_type = MockBuffer;
    using serializer_type = MockSerializer;

    bool testMessage(int level) { return true; }
    int argThreshold() const { return 3; }

    // caller must be able to go "give me a buffer"
    buffer_type acquireBuffer() {
      ++buffersRequested;
      return buffer_type();
    }

    // caller must be able to go "ok, flush this message buffer (and take it back if you want it)
    void submitMessage(buffer_type buffer) { ops.emplace_back("submitMessage", buffer.id); }
    // caller must be able to return ownership of the buffer
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
        using arg_type = std::tuple<std::string_view, int, int, std::string_view>;
        const auto [file, line, level, msg] = std::any_cast<arg_type>(std::get<1>(ops.at(1)));
        CHECK(file == "file");
        CHECK(line == 3);
        CHECK(level == 1);
        CHECK(msg == "hello");
    }
    CHECK(std::get<0>(ops.at(2)) == "epilogue");
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
}
