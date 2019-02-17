#ifndef NP_SERIALIZER_HPP
#define NP_SERIALIZER_HPP

#include <string_view>
#include <vector> //TODO: need real buffer type

namespace np {
  struct Serializer {
    using buffer_type = std::vector<char>;
    explicit Serializer(buffer_type* buffer);
    virtual ~Serializer() = default;

    virtual void prologue(std::string_view file, int line, int level, std::string_view msg);

    virtual void epilogue();

    virtual void writeKey(std::string_view name);

    virtual void write(double val);
    virtual void write(long double val);
    virtual void write(int val);
    virtual void write(unsigned int val);
    virtual void write(long long val);
    virtual void write(unsigned long long val);
    virtual void write(std::string_view val);
    virtual void write(bool val);
    virtual void writeRawJson(std::string_view val);

  private:
    struct JsonBuilder {
      void beginObject();
      void endObject();
      void beginArray();
      void endArray();

      virtual void writeString();
      virtual void writeNumber();
      virtual void writeBool();

      template <typename T>
      size_t maxLength(const T& val){

      }
    };

  private:
    void writeEscaped(std::string_view val);
    template <typename T>
    void writeNumber(T val, const char* format) noexcept;
    void writeLiteral(std::string_view val);

    buffer_type* buffer;
    int args_count = 0;
  };
}
#endif
