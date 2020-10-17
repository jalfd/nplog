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
      std::string_view log_name)
      : message_buffer(std::move(buffer)), serializer(&message_buffer), message_level(level) {
      serializer.prologue(file, line, level, log_name, m);
    }

    void endMessage() { serializer.epilogue(); }

    template <typename T>
    bool addParam(const char* name, T&& expr) {
      serializer.writeKey(name);
      auto vs = serializer.valueSerializer();
      np::log::format(expr, vs);
      return true;
    }

    const MessageBuffer& buffer() { return message_buffer; }

  protected:
    MessageBuffer message_buffer;
    Serializer serializer;

    level_type message_level;
  };

  struct NPLOG_EXPORT ScopedMessage : private ScopedMessageBase {
    ScopedMessage(Logger& log,
      const char* file,
      int line,
      level_type level,
      const char* m)
      : ScopedMessageBase(file, line, level, m, log.acquireBuffer(), log.name())
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
