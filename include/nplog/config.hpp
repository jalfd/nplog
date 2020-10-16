#ifndef NP_CONFIG_HPP
#define NP_CONFIG_HPP

#include <nplog/common.hpp>
#include <nplog/export.hpp>
#include <functional>
#include <iterator>
#include <map>
#include <string>
#include <vector>
#include <string_view>
#include <algorithm>

namespace np::log {
    struct MessageInfo {
        level_type level;
        std::string_view message;
    };
}

template class NPLOG_EXPORT std::function<void(np::log::MessageInfo msg)>;

namespace np::log {
  struct LevelSpec {
    level_type message = {};
    level_type param = {};
  };

  struct NPLOG_EXPORT Config {
    using Sink = std::function<void(MessageInfo)>;

    enum Fields : uint32_t {
      File = 1,
      Line = 2,
      Time = 4,
      Level = 8,
      LevelName = 16,
      LogName = 32,
      ProcessName = 64,
      ProcessId = 128,
      ThreadId = 256,
      Hostname = 512,
    };

    struct Levels {
        LevelSpec default_level;
        std::map<int, LevelSpec> levels_by_depth;
        std::map<std::string, LevelSpec> levels_by_name;
    };

    Sink sink;
    Fields fields = static_cast<Fields>(File | Line | Time | LevelName);
    Levels levels;
    // Some messages or parameters may be marked sensitive, and will only be logged when this flag is set
    bool sensitive_enabled = false;
  };
  namespace internal {
    struct LevelRule {
      std::string_view name;
      int depth;
      LevelSpec level;
    };

    NPLOG_EXPORT void
    applyConfig(Config::Sink sink, Config::Fields fields, bool sensitive, LevelSpec default_level, LevelRule* first, LevelRule* last);
  } // namespace internal


  inline void applyConfig(Config config)
  {
      std::vector<internal::LevelRule> level_rules;

      std::transform(config.levels.levels_by_depth.begin(),
        config.levels.levels_by_depth.end(),
        std::back_inserter(level_rules),
        [](const auto& rule) {
          return internal::LevelRule{{}, rule.first, rule.second};
        });

      std::transform(config.levels.levels_by_name.begin(),
        config.levels.levels_by_name.end(),
        std::back_inserter(level_rules),
        [](const auto& rule) {
          return internal::LevelRule{rule.first, 0, rule.second};
        });

      internal::applyConfig(config.sink, config.fields, config.sensitive_enabled, config.levels.default_level, &level_rules[0], &level_rules[level_rules.size()]);
  }
} // namespace np
#endif
