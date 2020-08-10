#ifndef NP_LOG_LOGGER_HPP
#define NP_LOG_LOGGER_HPP

#include <nplog/config.hpp>
#include <nplog/export.hpp>
#include <nplog/formatter.hpp>
#include <nplog/messagebuffer.hpp>
#include <nplog/serializer.hpp>
#include <iostream>

#include <string_view>

namespace np::log {
    // FIXME: can't use serializer_type typedef. Should we? Or get rid of? Or reference Logger::serializer_type?
  struct LogParam { // FIXME: move to detail
    template <typename T>
    LogParam(const char* name, T&& value)
      : name(name), func([=](Serializer& serializer, MessageBuffer& buffer) {
        serializer.writeKey(name);
        const auto name_end = buffer.messageSize(); // FIXME: do we really want to depend on buffer?
        auto vs = serializer.valueSerializer();
        np::log::format(value, vs);
        return name_end;
      }) {}
    const char* name;
    std::function<uint32_t(Serializer&, MessageBuffer&)> func;
  };
  struct NPLOG_EXPORT LoggerParams {
      // FIXME: note, this whole thing could live in the .cc file
    LoggerParams() = default;
    LoggerParams(LoggerParams* parent, std::initializer_list<LogParam> params);

    MessageBuffer data;
    std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> offsets; // FIXME: put in a struct, first, name last, last
  };

  struct NPLOG_EXPORT Logger {
    using buffer_type = MessageBuffer;
    using serializer_type = Serializer;

    explicit Logger(Logger* parent = nullptr, const char* name = nullptr);
    explicit Logger(const char* name);

    explicit Logger(Logger* parent, const char* name, std::initializer_list<LogParam> params)
      : Logger(parent, name) {
          logger_params = new LoggerParams(parent ? parent->logger_params : nullptr, params);
      }

    Logger(const Logger&) = delete;

    buffer_type acquireBuffer();

    ~Logger();

    void submitMessage(level_type level, buffer_type& buffer);

    void releaseBuffer(buffer_type&& buf);

    LevelSpec refreshLevels(unsigned version, bool exclude_depth = false);

    unsigned knownVersion() const { return version; }

    std::string_view name() const { return std::string_view(name_ptr, name_len); }

    bool permitSensitive() const { return sensitive; }

    LoggerParams* loggerParams() { return logger_params; } // FIXME: access hack
  private:
    LevelSpec effective_levels;
    LevelSpec levels_by_name_only;
    Logger* parent = nullptr;
    LoggerParams* logger_params = nullptr;
    const char* name_ptr = nullptr;
    size_t name_len = 0;
    const unsigned depth = 0;
    unsigned version = 0;
    bool sensitive = false;
  };
} // namespace np::log

namespace np {
  using log::Logger;
}
#endif
