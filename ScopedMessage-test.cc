#include "ScopedMessage.hpp"
#include "test-utils.hpp"
#include <sstream>
#include <vector>

// convenience operator for catch to use
bool operator==(const std::vector<char>& result, const std::string& expected);
#include <catch.hpp>

static std::vector<char> output;
namespace np {
  namespace {
    struct MockSerializer {
      using buffer_type = std::vector<char>;
      explicit MockSerializer(buffer_type& buffer) : buffer(buffer) {}

      void prologue(std::string_view, int line, int level, std::string_view msg);
      void epilogue();

      void writeNumber(int val);
      void writeNumber(unsigned int val);
      void writeNumber(int64_t val);
      void writeNumber(uint64_t val);
      void writeNumber(double val);

      void writeString(std::string_view val);
      void writeBool(bool val);
      void writeRawJson(std::string_view val);

    private:
      buffer_type& buffer;
    };

  struct Log {
    // FIXME: should this be wrapped in a unique ptr? It'd be an extra template instantiation, but would let us ensure no copies are accidentally made
    using buffer_type = std::vector<char>;
    using serializer_type = Serializer<buffer_type>;

    bool testMessage(int level);

    // caller must be able to go "give me a buffer"
    buffer_type acquireBuffer();

    // caller must be able to go "ok, flush this message buffer (and take it back if you want it)
    void submitMessage(buffer_type buffer);
    // caller must be able to return ownership of the buffer
    void releaseBuffer(buffer_type buffer);

  private:
    std::mutex buffer_mutex;
    std::vector<buffer_type> buffers;
  };
    struct MockLog {
      using buffer_type = std::vector<char>;
      using serializer_type = MockSerializer<std::back_insert_iterator<std::vector<char>>>;

      bool testMessage(int level);

      // caller must be able to go "give me a buffer"
      buffer_type acquireBuffer() {
        ++buffersRequested;
        return std::vector<char>();
      }

      // caller must be able to go "ok, flush this message buffer (and take it back if you want it)
      void submitMessage(buffer_type buffer) { output = buffer; }
      // caller must be able to return ownership of the buffer
      void releaseBuffer(buffer_type buffer);

      static inline int buffersRequested = 0;
    };

  } // namespace
} // namespace np

bool operator==(const std::vector<char>& result, const std::string& expected) {
    std::vector<char> e(expected.begin(), expected.end());
    return result == e;
}

TEST_CASE("ScopedMessage") {
  np::Log::buffersRequested = 0;
  np::Log log;

  SECTION("ScopedMessage requests a buffer") {
    np::ScopedMessage msg(log, "", 0, 0, "hello");
    CHECK(np::Log::buffersRequested == 1);
  }

  SECTION("ScopedMessage writes its header to the buffer") {
    { np::ScopedMessage msg(log, "file", 0, 1, "hello"); }
    // this should ask for a buffer, and fill it appropriately
    CHECK(np::Log::buffersRequested == 1);
    CHECK(output == "file|0|1|hello");
  }

  SECTION("ScopedMessage writes arguments to the buffer") {
    {
      np::ScopedMessage msg(log, "", 0, 0, "");
      output.clear();
      msg.addArg("name", 42.0);
    }
    CHECK(np::Log::buffersRequested == 1);
    CHECK(output == "|0|0|>name|42");
    // TODO: arg should be passed through formatter in addition to the serializer that other parts go through
  }

  SECTION("TODO: Somehow ensure that if I define a format specialization after defining ScopedMessage, the specialization is picked up") {

  }
}
