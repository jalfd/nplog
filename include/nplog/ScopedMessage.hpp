#ifndef NP_LOG_SCOPEDMESSAGE_HPP
#define NP_LOG_SCOPEDMESSAGE_HPP

#include <string_view>
#include <nplog/Log.hpp>

// TODO: what happens in case of reentrancy?
// I guess it could just be a protocol where message ctor goes "can I have a buffer please?", and we
// either provide the one we have, or create a new one
// Also, need a reentrancy test then

namespace np {
  template <typename LogType>
  struct ScopedMessage {
    ScopedMessage(LogType& log, const char* file, int line, int level, const char* m)
      : log(log)
      , param_level(log.paramLevel())
      , message_buffer(log.acquireBuffer())
      , serializer(&message_buffer)
      , message_level(level) {
      serializer.prologue(file, line, level, m);
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

    bool suppressParam(int i) { return i > param_level; }
    bool suppressParam(const char* = nullptr) { return suppressParam(message_level); }

  private:
    LogType& log;
    int param_level;
    typename LogType::buffer_type message_buffer;
    typename LogType::serializer_type serializer;

    int message_level;
  };

} // namespace np
#endif
