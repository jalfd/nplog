#ifndef NP_CONFIGIMPL_HPP
#define NP_CONFIGIMPL_HPP
#include <nplog/Config.hpp>

namespace np {
  // I am logger N, I am nested at depth D, and my most recent configuration is from version V
  bool isCurrent(unsigned v);
  std::pair<Levels, unsigned> getLevels(std::string_view n, unsigned d);
} // namespace np
#endif