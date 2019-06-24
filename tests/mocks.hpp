#ifndef NP_LOG_MOCKS_HPP
#define NP_LOG_MOCKS_HPP
#include <nplog/Serializer.hpp>
#include <any>
#include <iostream>
#include <tuple>

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
    explicit MockSerializer(buffer_type* buffer) { ops.emplace_back("ctor", buffer->id); }
    ~MockSerializer() { ops.emplace_back("dtor", nullptr); }

    void prologue(std::string_view file, int line, int level, std::string_view msg) {
      ops.emplace_back("prologue", std::make_tuple(file, line, level, msg));
    }

    void epilogue() { ops.emplace_back("epilogue", nullptr); }

    void writeKey(std::string_view name) { ops.emplace_back("writeKey", name); }

    np::ValueSerializer valueSerializer() { return np::ValueSerializer{nullptr}; }

  private:
  };
} // namespace
#endif
