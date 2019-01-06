#ifndef NP_LOG_SCOPEDMESSAGE_HPP
#define NP_LOG_SCOPEDMESSAGE_HPP

#include <string_view>

// TODO: what happens in case of reentrancy?
// I guess it could just be a protocol where message ctor goes "can I have a buffer please?", and we
// either provide the one we have, or create a new one
// Also, need a reentrancy test then

namespace np {
  template <typename LogType>
  struct ScopedMessage {
    ScopedMessage(LogType& log, const char* file, int line, int level, const char* m)
      : log(log)
      , argThreshold(log.argThreshold())
      , scratch_buffer(log.acquireBuffer())
      , message_buffer(log.acquireBuffer())
      , message_level(level)
      , serializer(message_buffer) {
      serializer.prologue(file, line, level, m);
    }

    ~ScopedMessage() {
      serializer.epilogue();
      log.submitMessage(message_buffer);
      log.releaseBuffer(message_buffer);
      log.releaseBuffer(scratch_buffer);
    }

    template <typename T>
    bool addArg(const char* name, T&& expr) {
      serializer.writeKey(name);
      np::format(std::forward<T>(expr), scratch_buffer, serializer);
      return true;
    }

    bool testArg(int i) { return i < argThreshold; }
    bool testArg(const char* = nullptr) { return testArg(message_level); }

  private:
    LogType& log;
    uint32_t argThreshold;
    typename LogType::buffer_type scratch_buffer;
    typename LogType::buffer_type message_buffer;
    typename LogType::serializer_type serializer;

    int message_level;
  };

} // namespace np
#endif
