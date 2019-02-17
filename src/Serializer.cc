#include <nplog/Serializer.hpp>
#if defined(_WIN32) || defined(__linux__)
#define has_to_chars
#endif
#ifdef has_to_chars
#include <charconv>
#else
#include <xlocale.h>
#endif

namespace np {
  namespace {
#ifndef has_to_chars
    namespace internal {
      static struct ScopedLocale {
        ScopedLocale() : loc(newlocale(LC_ALL_MASK, "C", 0)) {}

        ~ScopedLocale() { freelocale(loc); }
        locale_t loc;
        ScopedLocale(const ScopedLocale&) = delete;
        ScopedLocale& operator=(const ScopedLocale&) = delete;
      } c_locale;
    } // namespace internal
#endif
  } // namespace

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
    args_count = 0;
    // TODO: hmm... this doesn't quite work
    // and it hasn't solved the problem of separating json structure from content
  }

  void Serializer::epilogue() {
    if (args_count > 0) { buffer->push_back('}'); }
    buffer->push_back('}');
  }

  void Serializer::writeKey(std::string_view name) {
    writeEscaped(name);
    buffer->push_back(':');
  }

  void Serializer::write(double val) { writeNumber(val, "%.12g"); }

  void Serializer::write(long double val) { writeNumber(val, "%.12Lg"); }

  void Serializer::write(int val) { writeNumber(val, "%d"); }

  void Serializer::write(unsigned int val) { writeNumber(val, "%u"); }
  void Serializer::write(int64_t val) { writeNumber(val, "%dll"); }

  void Serializer::write(uint64_t val) { writeNumber(val, "%ull"); }

  void Serializer::write(std::string_view val) {
    writeLiteral(args_count++ ? std::string_view(",", 1) : std::string_view(",params:{", 9));
    writeEscaped(val);
  }

  void Serializer::write(bool val) {
    writeLiteral(args_count++ ? std::string_view(",", 1) : std::string_view(",params:{", 9));
    writeLiteral(val ? "true" : "false");
  }

  void Serializer::writeRawJson(std::string_view val) {
    writeLiteral(args_count++ ? std::string_view(",", 1) : std::string_view(",params:{", 9));
    writeLiteral(val);
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

  template <typename T>
  void Serializer::writeNumber(T val, const char* format) noexcept {
    static constexpr int bufsize = 1024;
    char buf[bufsize];
#ifndef has_to_chars
    const auto len = snprintf_l(buf, 32, internal::c_locale.loc, format, val);
    if (len > bufsize) { std::abort(); }
#else
    const auto result = std::to_chars(buf, buf + 32, val);
    if (result.ec) { std::abort(); }
    const auto len = result.ptr - buf;
#endif

    writeLiteral(args_count++ ? std::string_view(",", 1) : std::string_view(",params:{", 9));
    writeLiteral(std::string_view(buf, len));
  }
} // namespace np
