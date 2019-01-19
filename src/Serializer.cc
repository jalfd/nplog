#include <nplog/Serializer.hpp>
#if defined(_WIN32) || defined(__linux__)
#define has_to_char
#endif
#ifdef has_to_char
#include <charconv>
#else
#include <xlocale.h>
#endif

namespace np {
  namespace internal {
#ifndef has_to_char
    static struct ScopedLocale {
      ScopedLocale() : loc(newlocale(LC_ALL_MASK, "C", 0)) {}

      ~ScopedLocale() { freelocale(loc); }
      locale_t loc;
      ScopedLocale(const ScopedLocale&) = delete;
      ScopedLocale& operator=(const ScopedLocale&) = delete;
    } c_locale;
#endif
  } // namespace internal

  template <typename T>
  void writeFormat(T val, const char* format, Serializer& srl) {
    char buf[32];
#ifndef has_to_char
    const auto len = snprintf_l(buf, 32, internal::c_locale.loc, format, val);
#else
    const auto result = std::to_chars(buf, buf + 32, val);
    if (result.ec) { std::terminate(); }
    const auto len = result.ptr - buf;
#endif
    srl.writeLiteral(std::string_view(buf, len));
    srl.writePending(",");
  }

  Serializer::Serializer(buffer_type* buffer) : buffer(buffer) {}

  void Serializer::prologue(std::string_view file, int line, int level, std::string_view msg) {
    buffer->push_back('{');
    writeKey("file");
    write(file);
    writeKey("line");
    write(line);
    writeKey("level");
    write(level);
    writeKey("message");
    write(msg);
    buffer->resize(buffer->size() - pending_length);
    writePending(",\"params\":{");
  }

  void Serializer::epilogue() {
    buffer->resize(buffer->size() - pending_length);
    if (pending_length == 1) { buffer->push_back('}'); }
    buffer->push_back('}');
  }

  void Serializer::writeKey(std::string_view name) {
    writeEscaped(name);
    buffer->push_back(':');
  }

  void Serializer::write(double val) { writeFormat(val, "%f", *this); }

  void Serializer::write(int val) { writeFormat(val, "%d", *this); }

  void Serializer::write(unsigned int val) { writeFormat(val, "%u", *this); }
  void Serializer::write(int64_t val) { writeFormat(val, "%dll", *this); }

  void Serializer::write(uint64_t val) { writeFormat(val, "%ull", *this); }

  void Serializer::write(std::string_view val) {
    writeEscaped(val);
    writePending(",");
  }

  void Serializer::write(bool val) {
    writeLiteral(val ? "true" : "false");
    writePending(",");
  }

  void Serializer::writeRawJson(std::string_view val) {
    writeLiteral(val);
    writePending(",");
  }

  void Serializer::writeEscaped(std::string_view val) {
    buffer->push_back('"');
    std::for_each(val.begin(), val.end(), [this](char c) {
      switch (c) {
      case '"':
        buffer->push_back('\\');
        buffer->push_back('"');
        return;
      case '\\':
        buffer->push_back('\\');
        buffer->push_back('\\');
        return;
      case '\n':
        buffer->push_back('\\');
        buffer->push_back('n');
        return;
      case '\r':
        buffer->push_back('\\');
        buffer->push_back('r');
        return;
      case '\t':
        buffer->push_back('\\');
        buffer->push_back('t');
        return;
      default:
        if (c < 0x20) {
          buffer->push_back('\\');
          buffer->push_back('u');
          buffer->push_back('0');
          buffer->push_back('0');
          buffer->push_back(c < 0x10 ? '0' : '1');
          buffer->push_back(c & 0xf);
        } else {
          buffer->push_back(c);
        }
      }
    });
    buffer->push_back('"');
  }

  void Serializer::writeLiteral(std::string_view val) {
    std::copy(val.begin(), val.end(), std::back_inserter(*buffer));
  }

  void Serializer::writePending(std::string_view val) {
    std::copy(val.begin(), val.end(), std::back_inserter(*buffer));
    pending_length = val.size();
  }
}
