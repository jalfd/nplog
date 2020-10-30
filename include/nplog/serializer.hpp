#ifndef NP_SERIALIZER_HPP
#define NP_SERIALIZER_HPP

#include <nplog/common.hpp>
#include <nplog/export.hpp>
#include <string_view>

namespace np::log {
  struct MessageBuffer;
  struct NPLOG_EXPORT ValueSerializer {
    explicit ValueSerializer(MessageBuffer* buffer);
    ~ValueSerializer() = default;

    void write(double val);
    void write(long double val);
    void write(int val);
    void write(unsigned int val);
    void write(long val);
    void write(unsigned long val);
    void write(long long val);
    void write(unsigned long long val);
    void write(std::string_view val);
    void write(const char* val);
    void write(bool val);
    void writeLiteral(std::string_view val);

  private:
    void writeString(std::string_view val);
    template <typename T>
    void writeInteger(T val) noexcept;
    template <typename T>
    void writeFloatingPoint(T val, const char* format) noexcept;

    MessageBuffer* buffer;
  };

  struct NPLOG_EXPORT Serializer {
    explicit Serializer(MessageBuffer* buffer);
    ~Serializer() = default;

    void prologue(std::string_view file,
      int line,
      Fields enabled_fields,
      level_type level,
      std::string_view log_name,
      std::string_view msg);
    void epilogue();
    void writeKey(std::string_view name);

    ValueSerializer valueSerializer();

    void startObject(std::string_view group_name);
    void endObject();

  private:
    MessageBuffer* buffer = nullptr;
    bool is_empty = true;

    friend struct HeaderFields;
  };
} // namespace np::log
#endif
