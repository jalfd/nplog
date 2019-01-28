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

    // TODO: should not be public, not part of the public interface
    void writeLiteral(std::string_view val);
    void writePending(std::string_view val);
  private:
    void writeEscaped(std::string_view val);

    buffer_type* buffer;
    size_t pending_length = 0;
  };
}
#endif
