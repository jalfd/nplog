#ifndef NP_FORMATTER_HPP
#define NP_FORMATTER_HPP

#include <type_traits>
#include <string_view>

namespace np {
  struct ValueSerializer;
  template <typename T>
  struct Formatter
  {};

  template <typename T>
  inline void format(T&& val, ValueSerializer& srl) {
    Formatter<std::remove_reference_t<T>>()(std::forward<T>(val), srl);
  }
  void format(bool val, ValueSerializer& srl);
  void format(short val, ValueSerializer& srl);
  void format(unsigned short val, ValueSerializer& srl);
  void format(int val, ValueSerializer& srl);
  void format(unsigned int val, ValueSerializer& srl);
  void format(long val, ValueSerializer& srl);
  void format(unsigned long val, ValueSerializer& srl);
  void format(long long val, ValueSerializer& srl);
  void format(unsigned long long val, ValueSerializer& srl);
  void format(float val, ValueSerializer& srl);
  void format(double val, ValueSerializer& srl);
  void format(long double val, ValueSerializer& srl);
  void format(std::string_view val, ValueSerializer& srl);

} // namespace np

#endif
