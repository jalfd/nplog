#ifndef NP_LOG_LOG_HPP
#define NP_LOG_LOG_HPP
#include <algorithm>
#include <mutex>
#include <string_view>
#include <vector>
#if defined(_WIN32) || defined(__linux__)
#define has_to_char
#endif
#ifdef has_to_char
#include <charconv>
#else
#include <xlocale.h>
#endif
#include <iostream>

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

  // contract: User may specialize template, but may not overload
  // I may add overloads
  // Rationale: overloads are not dependent, so they're resolved on template definition, not
  // instantiation
  // TODO: think about handling standard types as non-template overloads, so we can push them to .cc
  // files
  template <typename T>
  void format(T&& val, Serializer& srl) {
    srl.write(val);
  }

  struct Log {
    // FIXME: should this be wrapped in a unique ptr? It'd be an extra template instantiation, but
    // would let us ensure no copies are accidentally made
    using buffer_type = std::vector<char>;
    using serializer_type = Serializer;

    bool testMessage(int level);
    int argThreshold() const;

    // caller must be able to go "give me a buffer"
    buffer_type acquireBuffer();

    // caller must be able to go "ok, flush this message buffer
    void submitMessage(buffer_type buffer);

    // caller must be able to return ownership of the buffer
    void releaseBuffer(buffer_type buf);

  private:
    // TODO: this should be pimpl'ed
    std::mutex buffer_mutex;
    std::vector<buffer_type> buffers;
  };
} // namespace np
#endif
