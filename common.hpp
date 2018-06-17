#ifndef NP_LOG_COMMON_HPP
#define NP_LOG_COMMON_HPP

#include <string_view>

namespace np::log {
  enum class Severity { All, Trace, Debug, Info, Warning, Error, Never };
  struct Header {
    std::string_view file;
    std::string_view line;
    Severity severity;
  };
  namespace internal {
    template <size_t LiteralLen>
    std::string_view literal(const char (&str)[LiteralLen]) {
      return std::string_view(str, LiteralLen-1);
    }
  } // namespace internal
} // namespace np::log
#endif
