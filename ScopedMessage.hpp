#ifndef NP_LOG_SCOPEDMESSAGE_HPP
#define NP_LOG_SCOPEDMESSAGE_HPP

struct LogType; //TODO: parametrize this properly
// template <typename LogType>
namespace np {
  struct ScopedMessage {
    ScopedMessage(const char*) {}

    using BufIter = char*;
    BufIter putIter() { return nullptr; }
    void putEnd(BufIter) {}

    // helpers to be used by args:

    inline bool testArg(int i); // test against log's settings
    inline bool testArg(const char* = nullptr); // assume arg uses the same debug level as the log message

    template <typename T>
    inline bool serialize(const char* name, const T& expr);

    const char* getName(const char* name, const char*) { return name; }
    const char* getName(int, const char* name) { return name; }

  private:
    int level;
//    LogType* log;
  };

#if 1
bool ScopedMessage::testArg(int i) { return i < 5; }
bool ScopedMessage::testArg(const char*) { return testArg(level);} // assume same debug level as the log message

template <typename T>
bool ScopedMessage::serialize(const char* name, const T& expr){ return true;}
#endif
} // namespace np
#endif
