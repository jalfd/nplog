#ifndef NP_FORMATTER_HPP
#define NP_FORMATTER_HPP

#include <string_view>

namespace np {
  struct Serializer;
  template <typename T>
  struct Formatter
  {
      void operator()(T&& val, Serializer& srl);
  };

  template <typename T>
  inline void format(T&& val, Serializer& srl) {
    Formatter<T>()(std::forward<T>(val), srl);
  }
  void format(bool val, Serializer& srl);
  void format(short val, Serializer& srl);
  void format(unsigned short val, Serializer& srl);
  void format(int val, Serializer& srl);
  void format(unsigned int val, Serializer& srl);
  void format(long val, Serializer& srl);
  void format(unsigned long val, Serializer& srl);
  void format(long long val, Serializer& srl);
  void format(unsigned long long val, Serializer& srl);
  void format(float val, Serializer& srl);
  void format(double val, Serializer& srl);
  void format(long double val, Serializer& srl);
  void format(std::string_view val, Serializer& srl);

} // namespace np

#endif
