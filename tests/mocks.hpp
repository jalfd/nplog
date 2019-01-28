#ifndef NP_LOG_MOCKS_HPP
#define NP_LOG_MOCKS_HPP
#include <nplog/Serializer.hpp>
#include <any>
#include <iostream>

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

  struct MockSerializer : np::Serializer {
    using buffer_type = MockBuffer;
    explicit MockSerializer(buffer_type* buffer) : np::Serializer(nullptr) {
      ops.emplace_back("ctor", buffer->id);
    }
    ~MockSerializer() override { ops.emplace_back("dtor", nullptr); }

    void prologue(std::string_view file, int line, int level, std::string_view msg) override {
      ops.emplace_back("prologue", std::make_tuple(file, line, level, msg));
    }

    void epilogue() override { ops.emplace_back("epilogue", nullptr); }

    void writeKey(std::string_view name) override { ops.emplace_back("writeKey", name); }

    void write(double val) override { ops.emplace_back("writeVal", val); }
    void write(long double val) override { ops.emplace_back("writeVal", val); }
    void write(int val) override { ops.emplace_back("writeVal", val); }
    void write(unsigned int val) override { ops.emplace_back("writeVal", val); }
    void write(int64_t val) override { ops.emplace_back("writeVal", val); }
    void write(uint64_t val) override { ops.emplace_back("writeVal", val); }
    void write(std::string_view val) override { ops.emplace_back("writeVal", val); }
    void write(bool val) override { ops.emplace_back("writeVal", val); }
    void writeRawJson(std::string_view val) override { ops.emplace_back("writeRawVal", val); }

  private:
  };

  inline void printOps() { // TODO: shouldn't need this
    for (const auto& op : ops) {
      std::cout << std::get<0>(op) << ": " << std::get<1>(op).type().name() << '\n';
    }
  }
} // namespace
#endif
