#include <sstream>
#include <vector>
#include "ScopedMessage.hpp"
#include "test-utils.hpp"

// convenience operator for catch to use
bool operator==(const std::vector<char>& result, const std::string& expected);
#include <catch.hpp>

static std::vector<char> output;
namespace np {
  namespace {
    struct MockLog {
      using buffer_type = std::vector<char>;
      using serializer_type = Serializer;

      bool testMessage(int level) { return true; }
      int argThreshold() const { return 3; }

      // caller must be able to go "give me a buffer"
      buffer_type acquireBuffer() {
        ++buffersRequested;
        return std::vector<char>();
      }

      // caller must be able to go "ok, flush this message buffer (and take it back if you want it)
      void submitMessage(buffer_type buffer) { output = buffer; }
      // caller must be able to return ownership of the buffer
      void releaseBuffer(buffer_type buffer) {}

      static inline int buffersRequested = 0;
    };

  } // namespace
} // namespace np

bool operator==(const std::vector<char>& result, const std::string& expected) {
  std::vector<char> e(expected.begin(), expected.end());
  return result == e;
}

TEST_CASE("ScopedMessage") {
  np::MockLog::buffersRequested = 0;
  np::MockLog log;

  SECTION("ScopedMessage requests a buffer") {
    np::ScopedMessage<np::MockLog> msg(log, "", 0, 0, "hello");
    CHECK(np::MockLog::buffersRequested == 1);
  }

  SECTION("ScopedMessage writes its header to the buffer") {
    { np::ScopedMessage<np::MockLog> msg(log, "file", 0, 1, "hello"); }
    // this should ask for a buffer, and fill it appropriately
    CHECK(np::MockLog::buffersRequested == 1);
    CHECK(output == "file|0|1|hello");
  }

  SECTION("ScopedMessage writes arguments to the buffer") {
    {
      np::ScopedMessage<np::MockLog> msg(log, "", 0, 0, "");
      output.clear();
      msg.addArg("name", 42.0);
    }
    CHECK(np::MockLog::buffersRequested == 1);
    CHECK(output == "|0|0|>name|42");
    // TODO: arg should be passed through formatter in addition to the serializer that other parts
    // go through
  }

  SECTION(
    "TODO: Somehow ensure that if I define a format specialization after defining ScopedMessage, "
    "the specialization is picked up") {}
}
