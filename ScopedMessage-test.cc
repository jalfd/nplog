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
    template <typename BufferIter>
    struct Serializer {
      Serializer(BufferIter it) : iter(it) {}
      void header(std::string_view file, int line, int level, std::string_view msg) {
          std::ostringstream ostr;
          ostr << file << '|' << line << '|' << level << '|' << msg;
          const auto str = ostr.str();
          std::copy(str.begin(), str.end(), iter);
      }

      void arg(std::string_view name, std::string_view value) {
          std::ostringstream ostr;
          ostr << '>' << name << '|' << value;
          const auto str = ostr.str();
          std::copy(str.begin(), str.end(), iter);
      }
      void arg(std::string_view name, double value) {
          std::ostringstream ostr;
          ostr << '>' << name << '|' << value;
          const auto str = ostr.str();
          std::copy(str.begin(), str.end(), iter);
      }
      void arg(std::string_view name, bool value) {
          std::ostringstream ostr;
          ostr << '>' << name << '|' << value;
          const auto str = ostr.str();
          std::copy(str.begin(), str.end(), iter);
      }
      void arg_literal(std::string_view name, std::string_view value) {
          std::ostringstream ostr;
          ostr << '>' << name << '|' << value;
          const auto str = ostr.str();
          std::copy(str.begin(), str.end(), iter);
      }

      BufferIter iter;
    };

    struct Log {
      static inline int buffersRequested = 0;

      using buffer_type = std::vector<char>;
      using serializer_type = Serializer<std::back_insert_iterator<std::vector<char>>>;

      buffer_type messageBuffer() {
          ++buffersRequested;
          return std::vector<char>();
      }

      template <typename T>
      void serialize(const char* name, const T& expr, buffer_type& buffer);
      void submitMessage(buffer_type buffer) { output = buffer; }

      uint32_t arg_threshold() const { return 0; }
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
    // arg should be passed through formatter in addition to the serializer that other parts go through
  }
}
