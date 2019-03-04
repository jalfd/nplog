#ifndef NP_CONFIG_HPP
#define NP_CONFIG_HPP

#include <nplog/export.hpp>
#include <string_view>
#include <memory>
#include <map>

#pragma warning( push )
#pragma warning( disable : 4251 ) //FIXME: hide the STL types behind pimpl

namespace np {
  struct Levels {
    int message = 0;
    int param = 0;
  };
  struct NPLOG_EXPORT Config {
    std::map<std::string, Levels> levels_by_name;
    std::map<unsigned, Levels> levels_by_depth;
    Levels default_levels;

    void apply() const;
  };

  inline bool suppressMessage(Levels lvl, int message) {
      return message > lvl.message;
  }
} // namespace np
#pragma warning( pop )
#endif
