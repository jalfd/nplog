#ifndef NP_FORMATTER_HPP
#define NP_FORMATTER_HPP

#include <nplog/export.hpp>
#include <string_view>
#include <type_traits>

namespace np {
  struct ValueSerializer;
  template <typename T>
  struct Formatter {};

  template <typename T>
  inline void format(const T& val, ValueSerializer& srl) {
    Formatter<T>()(val, srl);
  }
  NPLOG_EXPORT void format(bool val, ValueSerializer& srl);
  NPLOG_EXPORT void format(short val, ValueSerializer& srl);
  NPLOG_EXPORT void format(unsigned short val, ValueSerializer& srl);
  NPLOG_EXPORT void format(int val, ValueSerializer& srl);
  NPLOG_EXPORT void format(unsigned int val, ValueSerializer& srl);
  NPLOG_EXPORT void format(long val, ValueSerializer& srl);
  NPLOG_EXPORT void format(unsigned long val, ValueSerializer& srl);
  NPLOG_EXPORT void format(long long val, ValueSerializer& srl);
  NPLOG_EXPORT void format(unsigned long long val, ValueSerializer& srl);
  NPLOG_EXPORT void format(float val, ValueSerializer& srl);
  NPLOG_EXPORT void format(double val, ValueSerializer& srl);
  NPLOG_EXPORT void format(long double val, ValueSerializer& srl);
  NPLOG_EXPORT void format(std::string_view val, ValueSerializer& srl);
  NPLOG_EXPORT void format(const std::string& val, ValueSerializer& srl);

} // namespace np

#endif
