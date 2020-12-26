#include <date/date.h>
#include <nplog/config.hpp>
#include <nplog/serializer.hpp>
#include "messagebuffer.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include "configimpl.hpp"
#include "platform.hpp"
#include "utility.hpp"
#if __has_include(<charconv>)
#include <charconv>
#else
#include <locale.h>
#endif

#ifdef _WIN32
#include <intrin.h>
#pragma intrinsic(_BitScanReverse)
#else
#include <strings.h>
#endif

#ifndef __cpp_lib_to_chars
#include "tostringhelper.hpp"
#else
  inline void padded_decimal_from(int number, char* first, char* last, char) noexcept {
    char buf[4];
    auto result = std::to_chars(buf, buf + 4, number);
    const auto len = result.ptr - buf;
    uint32_t bufi;
    memcpy(&bufi, buf, 4);
    bufi = bufi >> (4 - len);
    std::memcpy(first, &bufi, 4);
  }

  template <typename T>
  inline std::string_view decimal_from(T number, char* first, char* last) noexcept {
    return std::string_view(first, std::to_chars(first, last, number).ptr);
  }

#endif

namespace np::log {
  namespace {
    template <typename BufType>
    char* offset(BufType* ptr, size_t idx) noexcept {
      auto& buf = *ptr;
      return &buf[0] + idx;
    }
#ifndef __cpp_lib_to_chars
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
#endif

    const std::string level_names[]
      = {"<invalid>", "Fatal", "Error", "Warning", "Status", "Debug", "Debug", "Debug", "Trace"};
  } // namespace

  struct HeaderFields {
    HeaderFields(Serializer& srl) noexcept : buffer(srl.buffer), vs(srl.valueSerializer()) {}

    using sv = std::string_view;
    void file(sv filename, int, level_type, sv) noexcept {
      vs.writeLiteral(",\"file\":");
      vs.write(filenameFromPath(filename));
    }

    void line(sv, int line, level_type, sv) noexcept {
      vs.writeLiteral(",\"line\":");
      vs.write(line);
    }

    void time(sv, int, level_type, sv) noexcept {
      vs.writeLiteral(",\"time\":");
      auto now = std::chrono::system_clock::now();
      auto date = date::floor<date::days>(now);
      auto time
        = date::make_time(std::chrono::duration_cast<std::chrono::milliseconds>(now - date));
      auto ymd = date::year_month_day{date};
      // make sure the buffer has enough capacity
      auto* ptr = buffer->insertAt(26);
      *ptr++ = '"';
      padded_decimal_from(static_cast<unsigned int>(static_cast<int>(ymd.year())), ptr, ptr + 4, '0');
      ptr += 4;
      *ptr++ = '-';
      padded_decimal_from(static_cast<unsigned int>(ymd.month()), ptr, ptr + 2, '0');
      ptr += 2;
      *ptr++ = '-';
      padded_decimal_from(static_cast<unsigned int>(ymd.day()), ptr, ptr + 2, '0');
      ptr += 2;
      *ptr++ = 'T';
      padded_decimal_from(static_cast<unsigned int>(time.hours().count()), ptr, ptr + 2, '0');
      ptr += 2;
      *ptr++ = ':';
      padded_decimal_from(static_cast<unsigned int>(time.minutes().count()), ptr, ptr + 2, '0');
      ptr += 2;
      *ptr++ = ':';
      padded_decimal_from(static_cast<unsigned int>(time.seconds().count()), ptr, ptr + 2, '0');
      ptr += 2;
      *ptr++ = '.';
      padded_decimal_from(static_cast<unsigned int>(time.subseconds().count()), ptr, ptr + 3, '0');
      ptr += 3;
      *ptr++ = 'Z';
      *ptr++ = '"';
    }

    void level(sv, int, level_type level, sv) noexcept {
      vs.writeLiteral(",\"level\":");
      vs.write(level & 0xff);
    }

    void levelName(sv, int, level_type level, sv) noexcept {
      vs.writeLiteral(",\"levelString\":");
#ifdef _WIN32
      unsigned long index = 0;
      unsigned long lvl = level & 0xff;
      if (!_BitScanReverse(&index, lvl)) { index = 1; }
#else
      unsigned int lvl = level & 0xff;
      unsigned int index = static_cast<unsigned int>(ffs(static_cast<int>(lvl)));
      if (index == 0) { index = 1; }
#endif
      vs.write(level_names[index]);
    }
    void logName(sv, int, level_type, sv log_name) noexcept {
      if (!log_name.empty()) {
        vs.writeLiteral(",\"log\":");
        vs.write(log_name);
      }
    }
    void processName(sv, int, level_type, sv) noexcept {
      vs.writeLiteral(",\"process\":");
      vs.write(np::log::platform::executableName());
    }
    void processId(sv, int, level_type, sv) noexcept {
      vs.writeLiteral(",\"pid\":");
      vs.write(np::log::platform::processId());
    }
    void threadId(sv, int, level_type, sv) noexcept {
      vs.writeLiteral(",\"tid\":");
      vs.write(np::log::platform::threadId());
    }
    void hostname(sv, int, level_type, sv) noexcept {
      vs.writeLiteral(",\"host\":");
      vs.write(np::log::platform::hostname());
    }

  private:
    MessageBuffer* buffer = nullptr;
    ValueSerializer vs;
  };

  Serializer::Serializer(MessageBuffer* buffer) noexcept : buffer(buffer) {}

  void Serializer::prologue(std::string_view file,
    int line,
    Fields enabled_fields,
    level_type level,
    std::string_view log_name,
    std::string_view msg) noexcept {
    auto vs = valueSerializer();

    vs.writeLiteral("{\"message\":");
    vs.write(msg);

    HeaderFields hf(*this);

    if (enabled_fields & File) { hf.file(file, line, level, log_name); }
    if (enabled_fields & Line) { hf.line(file, line, level, log_name); }
    if (enabled_fields & Time) { hf.time(file, line, level, log_name); }
    if (enabled_fields & Level) { hf.level(file, line, level, log_name); }
    if (enabled_fields & LevelName) { hf.levelName(file, line, level, log_name); }
    if (enabled_fields & LogName) { hf.logName(file, line, level, log_name); }
    if (enabled_fields & ProcessName) { hf.processName(file, line, level, log_name); }
    if (enabled_fields & ProcessId) { hf.processId(file, line, level, log_name); }
    if (enabled_fields & ThreadId) { hf.threadId(file, line, level, log_name); }
    if (enabled_fields & Hostname) { hf.hostname(file, line, level, log_name); }
  }

  void Serializer::epilogue() noexcept { buffer->append('}'); }

  void Serializer::writeKey(std::string_view name) noexcept {
    auto vs = valueSerializer();
    if (!is_empty) { vs.writeLiteral(","); }
    is_empty = false;
    vs.write(name);
    buffer->append(':');
  }

  ValueSerializer Serializer::valueSerializer() noexcept { return ValueSerializer(buffer); }

  void Serializer::startObject(std::string_view name) noexcept {
    auto vs = valueSerializer();
    vs.writeLiteral(",\"");
    vs.writeLiteral(name);
    vs.writeLiteral("\":{");
    is_empty = true;
  }

  void Serializer::endObject() noexcept {
    buffer->append('}');
    is_empty = false;
  }

  ValueSerializer::ValueSerializer(MessageBuffer* buffer) noexcept : buffer(buffer) {}

  void ValueSerializer::write(double val) noexcept { writeFloatingPoint(val, "%.12g"); }

  void ValueSerializer::write(long double val) noexcept { writeFloatingPoint(val, "%.12Lg"); }

  void ValueSerializer::write(int val) noexcept { writeInteger(val); }

  void ValueSerializer::write(unsigned int val) noexcept { writeInteger(val); }

  void ValueSerializer::write(long long val) noexcept { writeInteger(val); }

  void ValueSerializer::write(unsigned long long val) noexcept { writeInteger(val); }

  void ValueSerializer::write(long val) noexcept { writeInteger(val); }

  void ValueSerializer::write(unsigned long val) noexcept { writeInteger(val); }

  void ValueSerializer::write(std::string_view val) noexcept { writeString(val); }

  void ValueSerializer::write(const char* val) noexcept { write(std::string_view(val)); }

  void ValueSerializer::write(bool val) noexcept { writeLiteral(val ? "true" : "false"); }

  void ValueSerializer::writeString(std::string_view val) noexcept {
    buffer->append('"');
    std::for_each(val.begin(), val.end(), [this](char c) {
      switch (c) {
      case '"':
        buffer->append('\\');
        buffer->append('"');
        return;
      case '\\':
        buffer->append('\\');
        buffer->append('\\');
        return;
      case '\n':
        buffer->append('\\');
        buffer->append('n');
        return;
      case '\r':
        buffer->append('\\');
        buffer->append('r');
        return;
      case '\t':
        buffer->append('\\');
        buffer->append('t');
        return;
      default:
        if (static_cast<unsigned char>(c) < 0x20) {
          buffer->append('\\');
          buffer->append('u');
          buffer->append('0');
          buffer->append('0');
          buffer->append(c < 0x10 ? '0' : '1');
          char b[2];
          snprintf(b, 2, "%x", (c & 0xf));
          buffer->append(b[0]);
        } else {
          buffer->append(c);
        }
      }
    });
    buffer->append('"');
  }

  void ValueSerializer::writeLiteral(std::string_view val) noexcept {
    char* at = buffer->insertAt(val.size());
    std::copy(val.begin(), val.end(), at);
  }

  template <typename T>
  void ValueSerializer::writeInteger(T val) noexcept {
    // ensure we have room for this type
    const auto cur_size = buffer->messageSize();
    const auto max_size = std::numeric_limits<T>::digits10 + 2;
    char* at = buffer->insertAt(max_size);
    const auto num_view = decimal_from(val, at, at + max_size);
    buffer->shrinkTo(cur_size + num_view.size());
    return;
  }

  template <typename T>
  void ValueSerializer::writeFloatingPoint(T val, const char* format) noexcept {
    if (!std::isfinite(val)) {
      writeLiteral("null");
      return;
    }

    static constexpr int bufsize = 1024;
    char buf[bufsize];
#ifndef __cpp_lib_to_chars
#ifdef __linux
    auto old_loc = uselocale(c_locale.loc);
    const auto len = snprintf(buf, bufsize, format, val);
    uselocale(old_loc);
#elif defined(_WIN32)
    const auto len = _snprintf_s_l(buf, bufsize, _TRUNCATE, format, c_locale.loc, val);
    if (len == -1) { std::abort(); }
#else
    const auto len = snprintf_l(buf, bufsize, c_locale.loc, format, val);
#endif
    if (len > bufsize) { std::abort(); }
#else
    (void) format;
    const auto result = std::to_chars(buf, buf + bufsize, val, std::chars_format::general, 12);
    if (result.ec == std::errc()) { std::abort(); }
    const auto len = result.ptr - buf;
#endif

    writeLiteral(std::string_view(buf, to_size_t_checked(len)));
  }
} // namespace np::log
