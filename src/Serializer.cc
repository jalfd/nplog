#include <nplog/Serializer.hpp>
#include <cstdlib>
#include <algorithm>
#if defined(_WIN32) || defined(__linux__)
//#define has_to_chars
#endif
#ifdef has_to_chars
#include <charconv>
#else
#include <locale.h>
#include <xlocale.h>
#include <cstdio>
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
    auto vs = valueSerializer();
    vs.writeLiteral("{\"file\":");
    vs.write(file);
    vs.writeLiteral(",\"line\":");
    vs.write(line);
    vs.writeLiteral(",\"level\":");
    vs.write(level);
    vs.writeLiteral(",\"message\":");
    vs.write(msg);
  }

  void Serializer::epilogue() {
    if (has_params) { buffer->push_back('}'); }
    buffer->push_back('}');
  }

  void Serializer::writeKey(std::string_view name) {
    auto vs = valueSerializer();
    if (!has_params) { vs.writeLiteral(",\"params\":{"); }
    else { vs.writeLiteral(","); }
    vs.writeString(name);
    buffer->push_back(':');
    has_params = true;
  }

  ValueSerializer Serializer::valueSerializer() { return ValueSerializer(buffer); }

  ValueSerializer::ValueSerializer(buffer_type* buffer) : buffer(buffer) {}

  void ValueSerializer::write(double val) { writeNumber(val, "%.12g"); }

  void ValueSerializer::write(long double val) { writeNumber(val, "%.12Lg"); }

  void ValueSerializer::write(int val) { writeNumber(val, "%d"); }

  void ValueSerializer::write(unsigned int val) { writeNumber(val, "%u"); }
  void ValueSerializer::write(long long val) { writeNumber(val, "%lld"); }

  void ValueSerializer::write(unsigned long long val) { writeNumber(val, "%llu"); }

  void ValueSerializer::write(std::string_view val) {
    writeString(val);
  }

  void ValueSerializer::write(bool val) {
    writeLiteral(val ? "true" : "false");
  }

  void ValueSerializer::writeRawJson(std::string_view val) {
    writeLiteral(val);
  }

  void ValueSerializer::writeString(std::string_view val) {
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

  void ValueSerializer::writeLiteral(std::string_view val) {
    std::copy(val.begin(), val.end(), std::back_inserter(*buffer));
  }

  template <typename T>
  void ValueSerializer::writeNumber(T val, const char* format) noexcept {
    static constexpr int bufsize = 1024;
    char buf[bufsize];
#ifndef has_to_chars
#ifdef __linux
    auto old_loc = uselocale(internal::c_locale.loc);
    const auto len = snprintf(buf, bufsize, format, val);
    uselocale(old_loc);
#else
    const auto len = snprintf_l(buf, bufsize, internal::c_locale.loc, format, val);
#endif
    if (len > bufsize) { std::abort(); }
#else
    const auto result = std::to_chars(buf, buf + bufsize, val);
    if (result.ec) { std::abort(); }
    const auto len = result.ptr - buf;
#endif

    writeLiteral(std::string_view(buf, len));
  }
} // namespace np
