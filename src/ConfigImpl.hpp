#ifndef NP_CONFIGIMPL_HPP
#define NP_CONFIGIMPL_HPP
#include <nplog/Config.hpp>
#include <nplog/export.hpp>

namespace np {
  // I am logger N, I am nested at depth D, and my most recent configuration is from version V
  bool isCurrent(unsigned v);

  struct LevelsResult {
    unsigned version = 0;
    Levels effective_levels;
    Levels levels_by_name_only;
  };
  NPLOG_EXPORT LevelsResult getLevels(std::string_view n, unsigned d);

  inline Levels merge(Levels lhs, Levels rhs) {
    return {std::max(lhs.message, rhs.message), std::max(lhs.param, rhs.param)};
  }
} // namespace np
#endif
