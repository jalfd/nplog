#include <nplog/config.hpp>
#include <nplog/serializer.hpp>
#include <algorithm>
#include <cstdlib>
#include "configimpl.hpp"
#include "platform.hpp"
#include "utility.hpp"
#include <locale.h>
#include <cmath>
#include <cstdio>
#include <date/date.h>
#include <chrono>
#include <limits>

#include "tostringhelper.hpp"

namespace np::log {
  namespace {
    template <typename BufType>
    char* offset(BufType* ptr, size_t idx) {
      auto& buf = *ptr;
      return &buf[0] + idx;
    }
#ifndef has_to_chars
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

    const std::map<level_type, std::string> level_names = {
      {static_cast<level_type>(0), "Fatal"},
      {static_cast<level_type>(1), "Error"},
      {static_cast<level_type>(2), "Warning"},
      {static_cast<level_type>(3), "Info"},
      {static_cast<level_type>(4), "Debug"},
      {static_cast<level_type>(5), "Trace"}};
  } // namespace

  struct HeaderFields {
    HeaderFields(Serializer& srl) : buffer(srl.buffer), vs(srl.valueSerializer()) {}

    using sv = std::string_view;
    void file(sv filename, int, level_type, sv) {
      vs.writeLiteral(",\"file\":");
      vs.write(filenameFromPath(filename));
    }

    void line(sv, int line, level_type, sv) {
      vs.writeLiteral(",\"line\":");
      vs.write(line);
    }

    void time(sv, int, level_type, sv) {
      vs.writeLiteral(",\"time\":");
      auto now = std::chrono::system_clock::now();
      auto date = date::floor<date::days>(now);
      auto time
        = date::make_time(std::chrono::duration_cast<std::chrono::milliseconds>(now - date));
      auto ymd = date::year_month_day{date};
      // make sure the buffer has enough capacity
      auto* ptr = buffer->insertAt(26);
      *ptr++ = '"';
      padded_decimal_from(static_cast<int>(ymd.year()), ptr, ptr + 4, '0');
      ptr += 4;
      *ptr++ = '-';
      padded_decimal_from(static_cast<unsigned int>(ymd.month()), ptr, ptr + 2, '0');
      ptr += 2;
      *ptr++ = '-';
      padded_decimal_from(static_cast<unsigned int>(ymd.day()), ptr, ptr + 2, '0');
      ptr += 2;
      *ptr++ = 'T';
      padded_decimal_from(time.hours().count(), ptr, ptr + 2, '0');
      ptr += 2;
      *ptr++ = ':';
      padded_decimal_from(time.minutes().count(), ptr, ptr + 2, '0');
      ptr += 2;
      *ptr++ = ':';
      padded_decimal_from(static_cast<unsigned long>(time.seconds().count()), ptr, ptr + 2, '0');
      ptr += 2;
      *ptr++ = '.';
      padded_decimal_from(static_cast<unsigned long>(time.subseconds().count()), ptr, ptr + 3, '0');
      ptr += 3;
      *ptr++ = 'Z';
      *ptr++ = '"';
    }

    void level(sv, int, level_type level, sv) {
      vs.writeLiteral(",\"level\":");
      vs.write(level & 0xff);
    }

    void levelName(sv, int, level_type level, sv) {
      vs.writeLiteral(",\"levelString\":");
      const auto lvl = level & 0xff;
      const auto it = level_names.find(static_cast<level_type>(lvl));
      if (it == level_names.end()) {
          vs.write(lvl);
      } else {
          vs.write(it->second);
      }
    }
    void logName(sv, int, level_type, sv log_name) {
      if (!log_name.empty()) {
        vs.writeLiteral(",\"log\":");
        vs.write(log_name);
      }
    }
    void processName(sv, int, level_type, sv) {
      vs.writeLiteral(",\"process\":");
      vs.write(np::log::platform::executableName());
    }
    void processId(sv, int, level_type, sv) {
      vs.writeLiteral(",\"pid\":");
      vs.write(np::log::platform::processId());
    }
    void threadId(sv, int, level_type, sv ) {
      vs.writeLiteral(",\"tid\":");
      vs.write(np::log::platform::threadId());
    }
    void hostname(sv, int, level_type, sv ) {
      vs.writeLiteral(",\"host\":");
      vs.write(np::log::platform::hostname());
    }

  private:
    Serializer::buffer_type* buffer = nullptr;
    ValueSerializer vs;
  };

  Serializer::Serializer(buffer_type* buffer) : buffer(buffer) {}

  void Serializer::prologue(std::string_view file,
    int line,
    level_type level,
    std::string_view log_name,
    std::string_view msg) {
    auto vs = valueSerializer();

    vs.writeLiteral("{\"message\":");
    vs.write(msg);

    HeaderFields hf(*this);

    Config::Fields enabled_fields = enabledFields();
    if (enabled_fields &  Config::File) { hf.file(file, line, level, log_name); }
    if (enabled_fields &  Config::Line) { hf.line(file, line, level, log_name); }
    if (enabled_fields &  Config::Time) { hf.time(file, line, level, log_name); }
    if (enabled_fields &  Config::Level) { hf.level(file, line, level, log_name); }
    if (enabled_fields &  Config::LevelName) { hf.levelName(file, line, level, log_name); }
    if (enabled_fields &  Config::LogName) { hf.logName(file, line, level, log_name); }
    if (enabled_fields &  Config::ProcessName) { hf.processName(file, line, level, log_name); }
    if (enabled_fields &  Config::ProcessId) { hf.processId(file, line, level, log_name); }
    if (enabled_fields &  Config::ThreadId) { hf.threadId(file, line, level, log_name); }
    if (enabled_fields &  Config::Hostname) { hf.hostname(file, line, level, log_name); }
  }

  void Serializer::epilogue() {
    buffer->append('}');
  }

  void Serializer::writeKey(std::string_view name) {
    auto vs = valueSerializer();
    if (!is_empty) {
      vs.writeLiteral(",");
    }
    is_empty = false;
    vs.write(name);
    buffer->append(':');
  }

  ValueSerializer Serializer::valueSerializer() { return ValueSerializer(buffer); }

  void Serializer::startObject(std::string_view name) {
      auto vs = valueSerializer();
      vs.writeLiteral(",\"");
      vs.writeLiteral(name);
      vs.writeLiteral("\":{");
      is_empty = true;
  }

  void Serializer::endObject(){
    buffer->append('}');
    is_empty = false;
  }

  ValueSerializer::ValueSerializer(buffer_type* buffer) : buffer(buffer) {}

  void ValueSerializer::write(double val) { writeNumber(val, "%.12g"); }

  void ValueSerializer::write(long double val) { writeNumber(val, "%.12Lg"); }

  void ValueSerializer::write(int val) { writeNumber(val, "%d"); }

  void ValueSerializer::write(unsigned int val) { writeNumber(val, "%u"); }

  void ValueSerializer::write(long long val) { writeNumber(val, "%lld"); }

  void ValueSerializer::write(unsigned long long val) { writeNumber(val, "%llu"); }

  void ValueSerializer::write(long val) { writeNumber(val, "%ld"); }

  void ValueSerializer::write(unsigned long val) { writeNumber(val, "%lu"); }

  void ValueSerializer::write(std::string_view val) { writeString(val); }

  void ValueSerializer::write(const char* val) { write(std::string_view(val)); }

  void ValueSerializer::write(bool val) { writeLiteral(val ? "true" : "false"); }

  void ValueSerializer::writeString(std::string_view val) {
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

  void ValueSerializer::writeLiteral(std::string_view val) {
    char* at = buffer->insertAt(val.size());
    std::copy(val.begin(), val.end(), at);
  }

  template <typename T>
  void ValueSerializer::writeNumber(T val, const char* format) noexcept {
    if constexpr (!std::is_floating_point_v<T>) {
      // ensure we have room for this type

      const auto cur_size = buffer->messageSize();
      const auto max_size = std::numeric_limits<T>::digits10 + 2;
      char* at = buffer->insertAt(max_size);
      const auto num_view = decimal_from(val, at, at + max_size);
      buffer->shrinkTo(cur_size + num_view.size());
      return;
    }

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
    const auto result = std::to_chars(buf, buf + bufsize, val);
    if (result.ec) { std::abort(); }
    const auto len = result.ptr - buf;
#endif

    writeLiteral(std::string_view(buf, to_size_t_checked(len)));
  }
} // namespace np
