#ifndef NP_SERIALIZER_HPP
#define NP_SERIALIZER_HPP

#include <string_view>
#include <vector> //TODO: need real buffer type

namespace np {
  struct Serializer {
    using buffer_type = std::vector<char>;
    explicit Serializer(buffer_type& buffer);

    void prologue(std::string_view file, int line, int level, std::string_view msg);

    void epilogue();

    void writeKey(std::string_view name);

    void write(double val);
    void write(int val);
    void write(unsigned int val);
    void write(int64_t val);
    void write(uint64_t val);
    void write(std::string_view val);
    void write(bool val);
    void writeRawJson(std::string_view val);

    void writeLiteral(std::string_view val);
    void writePending(std::string_view val);
  private:
    void writeEscaped(std::string_view val);

    buffer_type& buffer;
    size_t pending_length = 0;
  };
}
#endif
