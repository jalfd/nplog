#ifndef NP_SERIALIZER_HPP
#define NP_SERIALIZER_HPP

#include <nplog/common.hpp>
#include <nplog/export.hpp>
#include <string_view>
#include <nplog/messagebuffer.hpp>

namespace np {
  struct NPLOG_EXPORT ValueSerializer {
    using buffer_type = MessageBuffer;
    explicit ValueSerializer(buffer_type* buffer);
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
    void write(bool val);
    void writeLiteral(std::string_view val);

  private:
    void writeString(std::string_view val);
    template <typename T>
    void writeNumber(T val, const char* format) noexcept;

    buffer_type* buffer;
  };

  struct NPLOG_EXPORT Serializer {
    using buffer_type = MessageBuffer;
    explicit Serializer(buffer_type* buffer);
    ~Serializer() = default;

    void prologue(std::string_view file,
      int line,
      level_type level,
      std::string_view log_name,
      std::string_view msg);
    void epilogue();
    void writeKey(std::string_view name);

    ValueSerializer valueSerializer();

  private:
    buffer_type* buffer;
    bool has_params = false;

    friend struct HeaderFields;
  };
} // namespace np
#endif
