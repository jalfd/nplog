#ifndef NP_LOG_LOG_HPP
#define NP_LOG_LOG_HPP

#include "Serializer.hpp"
#include "Formatter.hpp"

#include <algorithm>
#include <functional>
#include <mutex>
#include <string_view>
#include <vector>
#if defined(_WIN32) || defined(__linux__)
#define has_to_char
#endif
#ifdef has_to_char
#include <charconv>
#else
#include <xlocale.h>
#endif
#include <iostream>

namespace np {
  struct Log {
    // FIXME: should this be wrapped in a unique ptr? It'd be an extra template instantiation, but
    // would let us ensure no copies are accidentally made
    using buffer_type = std::vector<char>;
    using serializer_type = Serializer;

    Log();

    bool suppressMessage(int level) const;
    int paramLevel() const;

    // caller must be able to go "give me a buffer"
    buffer_type acquireBuffer();

    // caller must be able to go "ok, flush this message buffer
    void submitMessage(int level, buffer_type& buffer);

    // caller must be able to return ownership of the buffer
    void releaseBuffer(buffer_type&& buf);

    static void setSink(std::function<void(int, std::string_view msg)> sink);
    static void setMessageLevel(int level);
    static void setParamLevel(int level);

  private:
    int message_level;
    int param_level;
  };

  std::function<void(int, std::string_view)> getStdErrSink();
} // namespace np
#endif
