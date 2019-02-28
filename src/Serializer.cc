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
#ifndef _WIN32
#include <xlocale.h>
#endif
#include <cstdio>
#include <cmath>
#endif
#include <date/date.h>
#include <chrono>

namespace np {
  namespace {
#ifndef has_to_chars
    namespace internal {
      static struct ScopedLocale {
#ifdef _WIN32
          using locale_t = _locale_t;
        ScopedLocale() : loc(_create_locale(LC_ALL, "C")) {}
        ~ScopedLocale() { _free_locale(loc); }
#else
        ScopedLocale() : loc(newlocale(LC_ALL_MASK, "C", 0)) {}
        ~ScopedLocale() { freelocale(loc); }
#endif

        locale_t loc;
        ScopedLocale(const ScopedLocale&) = delete;
        ScopedLocale& operator=(const ScopedLocale&) = delete;
      } c_locale;
    } // namespace internal
#endif
  } // namespace

  Serializer::Serializer(buffer_type* buffer) : buffer(buffer) {}

  void Serializer::prologue(std::string_view file, int line, int level, std::string_view msg) {
    const auto it
      = std::find_if(file.rbegin(), file.rend(), [](char c) { return c == '/' || c == '\\'; });
    if (it != file.rend()) {
        file.remove_prefix(file.rend() - it);
    }
    auto vs = valueSerializer();
    vs.writeLiteral("{\"file\":");
    vs.write(file);
    vs.writeLiteral(",\"line\":");
    vs.write(line);
    vs.writeLiteral(",\"time\":");
    auto now = std::chrono::system_clock::now();
    auto date = date::floor<date::days>(now);
    auto time = date::make_time(std::chrono::duration_cast<std::chrono::milliseconds>(now - date));
    auto ymd = date::year_month_day{date};
    const auto sz = buffer->size();
    buffer->resize(sz + 25);
    const auto written = snprintf(&(*buffer)[sz],
      25,
      "%04d-%02u-%02uT%02d:%02d:%02d.%03dZ",
      static_cast<int>(ymd.year()),
      static_cast<unsigned int>(ymd.month()),
      static_cast<unsigned int>(ymd.day()),
      static_cast<int>(time.hours().count()),
      static_cast<int>(time.minutes().count()),
      static_cast<int>(time.seconds().count()),
      static_cast<int>(time.subseconds().count())
      );
    buffer->resize(sz + written);
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
    vs.write(name);
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

  void ValueSerializer::write(long val) { writeNumber(val, "%ld"); }

  void ValueSerializer::write(unsigned long val) { writeNumber(val, "%lu"); }

  void ValueSerializer::write(std::string_view val) {
    writeString(val);
  }

  void ValueSerializer::write(bool val) {
    writeLiteral(val ? "true" : "false");
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
        if (static_cast<unsigned char>(c) < 0x20) {
          buffer->push_back('\\');
          buffer->push_back('u');
          buffer->push_back('0');
          buffer->push_back('0');
          buffer->push_back(c < 0x10 ? '0' : '1');
          char b[2];
          snprintf(b, 2, "%x", (c & 0xf));
          buffer->push_back(b[0]);
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
    if constexpr (std::is_floating_point_v<T>) {
        if (!std::isfinite(val)) {
            writeLiteral("null");
            return;
        }
    }
    static constexpr int bufsize = 1024;
    char buf[bufsize];
#ifndef has_to_chars
#ifdef __linux
    auto old_loc = uselocale(internal::c_locale.loc);
    const auto len = snprintf(buf, bufsize, format, val);
    uselocale(old_loc);
#elif defined(_WIN32)
    const auto len = _snprintf_s_l(buf, bufsize, _TRUNCATE, format, internal::c_locale.loc, val);
    if (len == -1) {
        std::abort();
    }
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
