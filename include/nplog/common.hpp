#ifndef NP_COMMON_HPP
#define NP_COMMON_HPP

#include <cstdint>

namespace np::log {
  using level_type = unsigned char;

  inline bool testLevel(level_type level, level_type threshold) { return level <= threshold; }
} // namespace np::log

#endif
