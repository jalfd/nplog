#ifndef NP_LOG_SCOPEDMESSAGE_HPP
#define NP_LOG_SCOPEDMESSAGE_HPP

#include <nplog/Log.hpp>
#include <string_view>

namespace np {
  template <typename LogType>
  struct ScopedMessage {
    ScopedMessage(LogType& log,
      const char* file,
      int line,
      level_type level,
      const char* m,
      level_type param_level)
      : log(log)
      , param_level(param_level)
      , message_buffer(log.acquireBuffer())
      , serializer(&message_buffer)
      , message_level(level)
      , permit_sensitive(log.permitSensitive()) {
      serializer.prologue(file, line, level, log.name(), m);
    }

    ~ScopedMessage() {
      serializer.epilogue();
      log.submitMessage(message_level, message_buffer);
      log.releaseBuffer(std::move(message_buffer));
    }

    template <typename T>
    bool addArg(const char* name, T&& expr) {
      serializer.writeKey(name);
      auto vs = serializer.valueSerializer();
      np::format(expr, vs);
      return true;
    }

    bool suppressParam(uint16_t i) {
      return !testLevel(static_cast<level_type>(i), static_cast<level_type>(param_level)) || (i >> 8) > static_cast<uint16_t>(permit_sensitive);
    }
    bool suppressParam(const char* = nullptr) { return suppressParam(message_level); }

  private:
    LogType& log;
    level_type param_level;
    typename LogType::buffer_type message_buffer;
    typename LogType::serializer_type serializer;

    level_type message_level;
    bool permit_sensitive;
  };

} // namespace np
#endif
