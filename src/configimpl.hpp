#ifndef NP_CONFIGIMPL_HPP
#define NP_CONFIGIMPL_HPP
#include <nplog/config.hpp>
#include <nplog/export.hpp>
#include <algorithm>
#include <shared_mutex>

namespace np::log {
  struct LogConfig {
    LogConfig() noexcept = default;
    LogConfig(LogConfig&& other) noexcept = default;
    LogConfig& operator=(LogConfig&& other) noexcept = default;

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

  void applyConfig(LogConfig cfg) noexcept;

  inline bool isCurrent(unsigned my_version, unsigned global_version) noexcept {
    return my_version == global_version;
  }

  struct LevelsResult {
    unsigned version = 0;
    LevelSpec effective_levels;
    LevelSpec levels_by_name_only;
  };
  NPLOG_EXPORT LevelsResult getLevels(std::string_view n, unsigned d) noexcept;

  inline LevelSpec merge(LevelSpec lhs, LevelSpec rhs) noexcept {
    return {std::max(lhs.message, rhs.message), std::max(lhs.props, rhs.props)};
  }

  Fields enabledFields(unsigned global_version) noexcept;

  void sendToSink(level_type level, std::string_view buffer, unsigned global_version) noexcept;
} // namespace np::log
#endif
