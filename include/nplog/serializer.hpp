#ifndef NP_SERIALIZER_HPP
#define NP_SERIALIZER_HPP

#include <nplog/common.hpp>
#include <nplog/export.hpp>
#include <string_view>

namespace np::log {
  struct MessageBuffer;
  struct NPLOG_EXPORT ValueSerializer {
    ValueSerializer() noexcept;
    explicit ValueSerializer(MessageBuffer* buffer) noexcept;
    ~ValueSerializer() noexcept = default;
    ValueSerializer(ValueSerializer&& other) noexcept;
    ValueSerializer& operator=(ValueSerializer&& other) noexcept;

    void write(double val) noexcept;
    void write(long double val) noexcept;
    void write(int val) noexcept;
    void write(unsigned int val) noexcept;
    void write(long val) noexcept;
    void write(unsigned long val) noexcept;
    void write(long long val) noexcept;
    void write(unsigned long long val) noexcept;
    void write(std::string_view val) noexcept;
    void write(const char* val) noexcept;
    void write(bool val) noexcept;
    void writeLiteral(std::string_view val) noexcept;

  private:
    void writeString(std::string_view val) noexcept;
    template <typename T>
    void writeInteger(T val) noexcept;
    template <typename T>
    void writeFloatingPoint(T val, const char* format) noexcept;

    MessageBuffer* buffer = nullptr;
  };

  struct NPLOG_EXPORT Serializer {
    explicit Serializer(MessageBuffer* buffer) noexcept;
    ~Serializer() noexcept = default;

    void prologue(std::string_view file,
      int line,
      Fields enabled_fields,
      level_type level,
      std::string_view log_name,
      std::string_view msg) noexcept;
    void epilogue() noexcept;
    void writeKey(std::string_view name) noexcept;

    ValueSerializer valueSerializer() noexcept;

    void startObject(std::string_view group_name) noexcept;
    void endObject() noexcept;

  private:
    MessageBuffer* buffer = nullptr;
    bool is_empty = true;

    friend struct HeaderFields;
  };
} // namespace np::log
#endif
