#ifndef NP_LOG_SCOPEDMESSAGE_HPP
#define NP_LOG_SCOPEDMESSAGE_HPP

#include <nplog/formatter.hpp>
#include <nplog/logger.hpp>
#include <string_view>

namespace np::log {
  struct NPLOG_EXPORT ScopedMessageBase {
    ScopedMessageBase(const char* file,
      int line,
      level_type level,
      const char* m,
      MessageBuffer buffer,
      std::string_view log_name,
      std::string_view logger_params_data)
      : message_buffer(std::move(buffer)), serializer(&message_buffer), message_level(level) {
      serializer.prologue(file, line, level, log_name, m);
      if (!logger_params_data.empty()) {
        serializer.startObject("static");
        serializer.valueSerializer().writeLiteral(logger_params_data);
        serializer.endObject();
      }
    }

    void endMessage() {
      if (has_params) { serializer.endObject(); }
      serializer.epilogue();
    }

    template <typename T>
    bool addParam(const char* name, T&& expr) {
      if (!has_params) {
        has_params = true;
        serializer.startObject("params");
      }
      serializer.writeKey(name);
      auto vs = serializer.valueSerializer();
      np::log::format(expr, vs);
      return true;
    }

    const MessageBuffer& buffer() { return message_buffer; }

  protected:
    level_type param_level;
    MessageBuffer message_buffer;
    Serializer serializer;

    level_type message_level;
    bool has_params = false;
  };

  struct NPLOG_EXPORT ScopedMessage : private ScopedMessageBase {
    ScopedMessage(Logger& log, const char* file, int line, level_type level, const char* m)
      : ScopedMessageBase(file,
        line,
        level,
        m,
        log.acquireBuffer(),
        log.name(),
        log.loggerParams() ? log.loggerParams()->data.contents() : std::string_view())
      , log(log) {}

    ~ScopedMessage() {
      endMessage();
      log.submitMessage(message_level, message_buffer);
      log.releaseBuffer(std::move(message_buffer));
    }

    using ScopedMessageBase::addParam;

  private:
    Logger& log;
  };
} // namespace np::log
#endif
