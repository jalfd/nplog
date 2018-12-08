#ifndef NP_LOG_LOG_HPP
#define NP_LOG_LOG_HPP

namespace np {
  struct Log {
    bool testMessage(int level);

    // caller must be able to go "give me a buffer"
    buffer_type messageBuffer();
    // caller must be able to go "please serialize this for me and put it in this buffer"
    template <typename T>
    void serialize(const char* name, const T& expr, buffer_type& buffer);
    // caller must be able to go "ok, flush this message buffer (and take it back if you want it)
    void submitMessage(buffer_type buffer);
  };

} // namespace np
#endif
