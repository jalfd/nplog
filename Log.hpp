#ifndef NP_LOG_LOG_HPP
#define NP_LOG_LOG_HPP
#include <string_view>
#include <mutex>
#include <vector>
#include <algorithm>
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
    namespace internal {
#ifndef has_to_char
        static struct ScopedLocale {
                ScopedLocale() :
                loc(newlocale(LC_ALL_MASK, "C", 0))
                {}

                ~ScopedLocale()
                {
                    freelocale(loc);
                }
                locale_t loc;
                ScopedLocale(const ScopedLocale&) = delete;
                ScopedLocale& operator=(const ScopedLocale&) = delete;
        } c_locale;
#endif
    }

    struct Serializer {
      using buffer_type = std::vector<char>;
      explicit Serializer(buffer_type& buffer) : buffer(buffer) {}

      void prologue(std::string_view file, int line, int level, std::string_view msg) {
        buffer.push_back('{');
        writeKey("file");
        write(file);
        writeKey("line");
        write(line);
        writeKey("level");
        write(level);
        writeKey("message");
        write(msg);
        buffer.resize(buffer.size() - pending_length);
        writePending(",\"params\":{");
    }

    void epilogue(){
        buffer.resize(buffer.size() - pending_length);
        if (pending_length == 1) {
            buffer.push_back('}');
        }
        buffer.push_back('}');
    }

    void writeKey(std::string_view name) {
        writeEscaped(name);
        buffer.push_back(':');
    }

    // put this in .cc file
    template <typename T>
    void writeFormat(T val, const char* format, Serializer& srl) {
      char buf[32];
#ifndef has_to_char
      const auto len = snprintf_l(buf, 32, internal::c_locale.loc, format, val);
#else
      const auto result = std::to_chars(buf, buf + 32, val);
      if (result.ec) {
          std::terminate();
      }
      const auto len = result.ptr - buf;
#endif
      srl.writeLiteral(std::string_view(buf, len));
      srl.writePending(",");
    }

    void write(double val) {
        writeFormat(val, "%f", *this);
    }

    void write(int val) {
        writeFormat(val, "%d", *this);
    }

    void write(unsigned int val) {
        writeFormat(val, "%u", *this);
    }
    void write(int64_t val) {
        writeFormat(val, "%dll", *this);
    }

    void write(uint64_t val) {
        writeFormat(val, "%ull", *this);
    }

    void write(std::string_view val) {
        writeEscaped(val);
        writePending(",");
    }

    void write(bool val) {
        writeLiteral(val ? "true" : "false");
        writePending(",");
    }

    void writeRawJson(std::string_view val) {
        writeLiteral(val);
        writePending(",");
    }

  private:
    void writeEscaped(std::string_view val) {
      buffer.push_back('"');
      std::for_each(val.begin(), val.end(), [this](char c) {
        switch (c) {
        case '"':
          buffer.push_back('\\');
          buffer.push_back('"');
          return;
        case '\\':
          buffer.push_back('\\');
          buffer.push_back('\\');
          return;
        case '\n':
          buffer.push_back('\\');
          buffer.push_back('n');
          return;
        case '\r':
          buffer.push_back('\\');
          buffer.push_back('r');
          return;
        case '\t':
          buffer.push_back('\\');
          buffer.push_back('t');
          return;
        default:
          if (c < 0x20) {
            buffer.push_back('\\');
            buffer.push_back('u');
            buffer.push_back('0');
            buffer.push_back('0');
            buffer.push_back(c < 0x10 ? '0' : '1');
            buffer.push_back(c & 0xf);
          } else {
            buffer.push_back(c);
          }
        }
      });
      buffer.push_back('"');
    }

    void writeLiteral(std::string_view val) {
        std::copy(val.begin(), val.end(), std::back_inserter(buffer));
    }
    void writePending(std::string_view val) {
        std::copy(val.begin(), val.end(), std::back_inserter(buffer));
        pending_length = val.size();
    }

    buffer_type& buffer;
    size_t pending_length = 0;
  };

  // contract: User may specialize template, but may not overload
  // I may add overloads
  // Rationale: overloads are not dependent, so they're resolved on template definition, not instantiation
  // TODO: think about handling standard types as non-template overloads, so we can push them to .cc files
  template <typename T>
  void format(T&& val, Serializer::buffer_type&, Serializer& srl) {
      srl.write(val);
  }

  struct Log {
    // FIXME: should this be wrapped in a unique ptr? It'd be an extra template instantiation, but would let us ensure no copies are accidentally made
    using buffer_type = std::vector<char>;
    using serializer_type = Serializer;

    bool testMessage(int level) { return level < 3; } // TODO: implement this properly
    int argThreshold() const { return 3; } // TODO: implement this properly

    // caller must be able to go "give me a buffer"
    buffer_type acquireBuffer() {
        std::lock_guard lock(buffer_mutex);
        if (buffers.empty()) {
            buffers.emplace_back();
        }
        const auto buf = buffers.back();
        buffers.pop_back();
        return buf;
    }

    // caller must be able to go "ok, flush this message buffer (and take it back if you want it)
    void submitMessage(buffer_type buffer) { std::cout << &buffer[0] << '\n'; }

    // caller must be able to return ownership of the buffer
    void releaseBuffer(buffer_type buf) {
        std::lock_guard lock(buffer_mutex);
        buffers.push_back(buf);
    }

  private:
    std::mutex buffer_mutex;
    std::vector<buffer_type> buffers;
  };

} // namespace np
#endif
