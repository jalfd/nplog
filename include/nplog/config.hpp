#ifndef NP_CONFIG_HPP
#define NP_CONFIG_HPP

#include <nplog/common.hpp>
#include <nplog/export.hpp>
#include <algorithm>
#include <functional>
#include <iterator>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace np::log {
  struct MessageInfo {
    level_type level;
    std::string_view message;
  };
} // namespace np::log

template class NPLOG_EXPORT std::function<void(np::log::MessageInfo msg)>;

namespace np::log {
  struct NPLOG_EXPORT Config {
    using Sink = std::function<void(MessageInfo)>;

    struct Levels {
      LevelSpec default_level = {threshold(Status), threshold(Status)};
      std::map<int, LevelSpec> levels_by_depth;
      std::map<std::string, LevelSpec> levels_by_name;
    };

    Sink sink;
    Fields fields = static_cast<Fields>(File | Line | Time | LevelName);
    Levels levels;
  };
  namespace internal {
    struct LevelRule {
      std::string_view name;
      int depth;
      LevelSpec level;
    };

    NPLOG_EXPORT void applyConfig(Config::Sink sink,
      Fields fields,
      LevelSpec default_level,
      LevelRule* first,
      LevelRule* last);
  } // namespace internal

  inline void applyConfig(Config config) {
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

    const auto has_rules = !level_rules.empty();
    internal::applyConfig(config.sink,
      config.fields,
      config.levels.default_level,
      has_rules ? &level_rules[0] : nullptr,
      has_rules ? &level_rules[0] + level_rules.size() : nullptr);
  }
} // namespace np::log
#endif
