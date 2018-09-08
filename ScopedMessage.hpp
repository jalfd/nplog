#ifndef NP_LOG_SCOPEDMESSAGE_HPP
#define NP_LOG_SCOPEDMESSAGE_HPP

#include <string_view>

// TODO: what happens in case of reentrancy?
// I guess it could just be a protocol where message ctor goes "can I have a buffer please?", and we
// either provide the one we have, or create a new one
// Also, need a reentrancy test then

// TODO: For exposition only
/*
template <typename BufferIter>
struct Serializer {
    Serializer(BufferIter it) : iter(it) {}
    void header(std::string_view file, int line, int level, std::string_view msg) {}
    template <typename T>
    void arg(std::string_view name, const T& expr) {}

    BufferIter iter;
};
*/

namespace np {
  template <typename LogType>
  struct ScopedMessage {
    ScopedMessage(LogType& log, const char* file, int line, int level, const char* m)
      : log(log)
      , arg_threshold(log.arg_threshold())
      , buffer(log.messageBuffer())
      , message_level(level) {
      // write header to buffer
      auto srl = typename LogType::serializer_type(std::back_inserter(buffer));
      srl.header(file, line, level, m);
    }

    ~ScopedMessage() { log.submitMessage(buffer); }

    template <typename T>
    bool addArg(const char* name, const T& expr) {
      auto srl = typename LogType::serializer_type(std::back_inserter(buffer));
      srl.arg(name, expr);
      //srl.arg(name, format(srl.iter(), expr));
      return true;
    }

    bool testArg(int i) { return i < arg_threshold; }
    bool testArg(const char* = nullptr) { return testArg(message_level); }

  private:
    LogType& log;
    uint32_t arg_threshold;
    typename LogType::buffer_type buffer;
    int message_level;
  };

} // namespace np
#endif
