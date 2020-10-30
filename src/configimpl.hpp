#ifndef NP_CONFIGIMPL_HPP
#define NP_CONFIGIMPL_HPP
#include <nplog/config.hpp>
#include <nplog/export.hpp>
#include <algorithm>
#include <shared_mutex>

namespace np::log {
  struct LogConfig {
    LogConfig() = default;
    LogConfig(LogConfig&& other) = default;
    LogConfig& operator=(LogConfig&& other) = default;

    using Sink = Config::Sink;
    using Fields = np::log::Fields;

    struct Levels {
      std::vector<char> name_data;
      LevelSpec default_level;
      std::vector<std::pair<std::string_view, LevelSpec>> levels_by_name;
      std::vector<LevelSpec> levels_by_depth;
    };
    Levels levels;
    Sink sink;
    Fields fields = static_cast<Fields>(File | Line | Time);
  };

  void applyConfig(LogConfig cfg);

  inline bool isCurrent(unsigned my_version, unsigned global_version) {
    return my_version == global_version;
  }

  struct LevelsResult {
    unsigned version = 0;
    LevelSpec effective_levels;
    LevelSpec levels_by_name_only;
  };
  NPLOG_EXPORT LevelsResult getLevels(std::string_view n, unsigned d);

  inline LevelSpec merge(LevelSpec lhs, LevelSpec rhs) {
    return {static_cast<level_type>(lhs.message | rhs.message),
      static_cast<level_type>(lhs.param | rhs.param)};
  }

  Fields enabledFields(unsigned global_version);

  void sendToSink(level_type level, std::string_view buffer, unsigned global_version);
} // namespace np::log
#endif
