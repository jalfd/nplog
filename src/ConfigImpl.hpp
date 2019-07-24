#ifndef NP_CONFIGIMPL_HPP
#define NP_CONFIGIMPL_HPP
#include <nplog/Config.hpp>
#include <nplog/export.hpp>
#include <algorithm>
#include <shared_mutex>

namespace np {
  struct LogConfig {
    LogConfig() = default;
    LogConfig(LogConfig&& other) = default;
    LogConfig& operator=(LogConfig&& other) = default;

    using Sink = Config::Sink;
    using Fields = Config::Fields;

    struct Levels {
      std::vector<char> name_data;
      LevelSpec default_level;
      std::vector<std::pair<std::string_view, LevelSpec>> levels_by_name;
      std::vector<LevelSpec> levels_by_depth;
      bool sensitive;
    };
    Levels levels;
    Sink sink;
    Fields fields;
  };

  void applyConfig(LogConfig cfg);

  // I am logger N, I am nested at depth D, and my most recent configuration is from version V
  bool isCurrent(unsigned v);

  struct LevelsResult {
    unsigned version = 0;
    LevelSpec effective_levels;
    LevelSpec levels_by_name_only;
    bool sensitive = false;
  };
  NPLOG_EXPORT LevelsResult getLevels(std::string_view n, unsigned d);

  inline LevelSpec merge(LevelSpec lhs, LevelSpec rhs) {
    return {std::max(lhs.message, rhs.message), std::max(lhs.param, rhs.param)};
  }

  Config::Fields enabledFields();

  void sendToSink(level_type level, std::string_view buffer);

} // namespace np
#endif
