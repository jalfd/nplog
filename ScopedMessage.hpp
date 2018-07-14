#ifndef NP_LOG_SCOPEDMESSAGE_HPP
#define NP_LOG_SCOPEDMESSAGE_HPP

struct LogType; //TODO: parametrize this properly
// template <typename LogType>
namespace np {
  struct ScopedMessage {
      ScopedMessage(const char* file, int line, int level, const char* m){}

    using BufIter = char*;
    BufIter putIter() { return nullptr; }
    void putEnd(BufIter) {}

    // helpers to be used by args:

    inline bool testArg(int i) { return i < 5; }
    inline bool testArg(const char* = nullptr) { return testArg(level); }

    template <typename T>
    inline bool addArg(const char* name, const T& expr){ return true;}
  private:
    int level;
//    LogType* log;
  };

} // namespace np
#endif
