#ifndef NP_LOG_MOCKS_HPP
#define NP_LOG_MOCKS_HPP
#include <nplog/serializer.hpp>
#include <any>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

namespace {
  // convenience operator for catch to use
  bool operator==(const std::vector<char>& result, const std::string& expected);

  using SerializerOp = std::tuple<std::string, std::any>;
  std::vector<SerializerOp> ops;

  struct MockBuffer {
    static inline int count = 0;
    MockBuffer() : id(++count) {}
    MockBuffer(MockBuffer&&) = default;

    const int id;
  };

  struct MockSerializer {
    using buffer_type = MockBuffer;
    explicit MockSerializer(buffer_type* buffer) : bid(buffer->id) {
      ops.emplace_back("ctor", bid);
    }
    ~MockSerializer() { ops.emplace_back("dtor", nullptr); }

    void prologue(std::string_view file,
      int line,
      np::log::level_type level,
      std::string_view log_name,
      std::string_view msg) {
      ops.emplace_back("prologue", std::make_tuple(file, line, level, log_name, msg, bid));
    }

    void epilogue() { ops.emplace_back("epilogue", bid); }

    void writeKey(std::string_view name) { ops.emplace_back("writeKey", name); }

    np::log::ValueSerializer valueSerializer() { return np::log::ValueSerializer{nullptr}; }

  private:
    int bid;
  };
} // namespace
#endif
