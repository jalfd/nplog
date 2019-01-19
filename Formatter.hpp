#ifndef NP_FORMATTER_HPP
#define NP_FORMATTER_HPP

namespace np {
  template <typename T>
  struct Formatter
  {
      void operator()(T&& val, Serializer& srl);
  };
}

#endif
