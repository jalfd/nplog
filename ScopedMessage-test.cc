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

struct MockSerializer {
    using buffer_type = int;
    explicit MockSerializer(buffer_type& buffer) {
        ops.emplace_back("ctor", &buffer);
    }

    void prologue(std::string_view file, int line, int level, std::string_view msg) {
        ops.emplace_back("prologue", std::make_tuple(file, line, level, msg));
    }

    void epilogue() {
        ops.emplace_back("dtor", nullptr);
    }

    void writeKey(std::string_view name) {
        ops.emplace_back("writeKey", name);
    }

    void write(double val) {
        ops.emplace_back("write", val);
    }
    void write(int val) {
        ops.emplace_back("write", val);
    }
    void write(unsigned int val);
    void write(int64_t val);
    void write(uint64_t val);
    void write(std::string_view val);
    void write(bool val);
    void writeRawJson(std::string_view val);
    private:
};

// ADL to the rescue
template <typename T>
void format(T&& val, MockSerializer::buffer_type&, MockSerializer& srl) {
    srl.write(-val);
}

namespace np {

  namespace {
    struct MockLog {
      using buffer_type = int;
      using serializer_type = MockSerializer;

      bool testMessage(int level) { return true; }
      int argThreshold() const { return 3; }

      // caller must be able to go "give me a buffer"
      buffer_type acquireBuffer() {
        ++buffersRequested;
        return buffer_type();
      }

      // caller must be able to go "ok, flush this message buffer (and take it back if you want it)
      void submitMessage(buffer_type buffer) {
          ops.emplace_back("submitMessage", &buffer);
      }
      // caller must be able to return ownership of the buffer
      void releaseBuffer(buffer_type buffer) {
          ops.emplace_back("releaseBuffer", &buffer);
      }

      int buffersRequested = 0;
    };

  } // namespace
} // namespace np

bool operator==(const std::vector<char>& result, const std::string& expected) {
  std::vector<char> e(expected.begin(), expected.end());
  return result == e;
}

void printOps() {
  for (const auto& op : ops) {
    std::cout << std::get<0>(op) << ": " << std::get<1>(op).type().name() << '\n';
  }
}

TEST_CASE("ScopedMessage") {
  ops.clear();
  np::MockLog log;

  SECTION("ScopedMessage requests a buffer") {
    np::ScopedMessage<np::MockLog> msg(log, "", 0, 0, "hello");
    CHECK(log.buffersRequested == 1);
  }

  SECTION("ScopedMessage writes its header to the buffer") {
    { np::ScopedMessage<np::MockLog> msg(log, "file", 0, 1, "hello"); }
    // this should ask for a buffer, and fill it appropriately
    CHECK(log.buffersRequested == 1);
    CHECK(std::get<0>(ops.at(0)) == "ctor");
    CHECK(std::get<0>(ops.at(1)) == "prologue");
    CHECK(std::get<0>(ops.at(2)) == "epilogue");
    printOps();
  }

  SECTION("ScopedMessage writes arguments to the buffer") {
    {
      np::ScopedMessage<np::MockLog> msg(log, "", 0, 0, "");
      msg.addArg("name", 42.0);
    }
    CHECK(log.buffersRequested == 1);
//    CHECK(output == "|0|0|>name|42");
    // TODO: arg should be passed through formatter in addition to the serializer that other parts
    // go through
  }

  SECTION(
    "TODO: Somehow ensure that if I define a format specialization after defining ScopedMessage, "
    "the specialization is picked up") {}
}
