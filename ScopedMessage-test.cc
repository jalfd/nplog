#include <sstream>
#include <vector>
#include "ScopedMessage.hpp"
#include "test-utils.hpp"

// convenience operator for catch to use
bool operator==(const std::vector<char>& result, const std::string& expected);
#include <catch.hpp>
#include<any> 

using SerializerOp = std::tuple<std::string, std::any>;
std::vector<SerializerOp> ops;

struct MockBuffer {
    static inline int count = 0;
    MockBuffer() : id(++count) {}
    MockBuffer(MockBuffer&&) = default;

    const int id;
};

struct MockSerializer : np::Serializer {
    using buffer_type = MockBuffer;
    explicit MockSerializer(buffer_type* buffer) : np::Serializer(nullptr) {
        ops.emplace_back("ctor", buffer->id);
    }
    ~MockSerializer() {
        ops.emplace_back("dtor", nullptr);
    }

    void prologue(std::string_view file, int line, int level, std::string_view msg) override {
        ops.emplace_back("prologue", std::make_tuple(file, line, level, msg));
    }

    void epilogue() override {
        ops.emplace_back("epilogue", nullptr);
    }

    void writeKey(std::string_view name) override {
        ops.emplace_back("writeKey", name);
    }

    void write(double val) override {}
    void write(int val) override {}
    void write(unsigned int val) override {}
    void write(int64_t val) override {}
    void write(uint64_t val) override {}
    void write(std::string_view val) override {
        ops.emplace_back("write", val);
    }
    void write(bool val) override {}
    void writeRawJson(std::string_view val) override {}
    private:
};

namespace test1 {
    struct Foo {};
}

namespace np {
  template <>
  struct Formatter<test1::Foo>
  {
      void operator()(test1::Foo&& val, Serializer& srl)
      {
          srl.write(std::string_view("Foo"));
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

void printOps() { // TODO: shouldn't need this
  for (const auto& op : ops) {
    std::cout << std::get<0>(op) << ": " << std::get<1>(op).type().name() << '\n';
  }
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
    {
      np::ScopedMessage<MockLog> msg(log, "", 0, 0, "");
      msg.addArg("name", test1::Foo());
    }
    CHECK(log.buffersRequested == 1);
    CHECK(ops.size() == 8);
    CHECK(std::get<0>(ops.at(0)) == "ctor");
    CHECK(std::get<0>(ops.at(1)) == "prologue");
    CHECK(std::get<0>(ops.at(2)) == "writeKey");
    CHECK(std::any_cast<std::string_view>(std::get<1>(ops.at(2))) == "name");
    CHECK(std::get<0>(ops.at(3)) == "write");
    CHECK(std::any_cast<std::string_view>(std::get<1>(ops.at(3))) == "Foo");
    CHECK(std::get<0>(ops.at(4)) == "epilogue");
    CHECK(std::get<0>(ops.at(5)) == "submitMessage");
    CHECK(std::get<0>(ops.at(6)) == "releaseBuffer");
    CHECK(std::get<0>(ops.at(7)) == "dtor");
  }
}
