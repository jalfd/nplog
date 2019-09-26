#include <nplog/config.hpp>
#include <algorithm>
#include <atomic>
#include <functional>
#include <map>
#include <numeric>
#include <shared_mutex>
#include <string>
#include <vector>
#include "configimpl.hpp"

namespace np::log {
  namespace internal {
    void applyConfig(Config::Sink sink, Config::Fields fields, bool sensitive, LevelSpec default_level, LevelRule* first, LevelRule* last) {
      LogConfig cfg;
      cfg.sink = sink;
      cfg.fields = fields;
      cfg.levels.default_level = default_level;
      cfg.levels.sensitive = sensitive;

      // so for each level rule
      const auto name_len = std::accumulate(
        first, last, size_t(), [](auto acc, auto rule) { return acc + rule.name.size(); });
      cfg.levels.name_data.reserve(name_len);

      std::for_each(first, last, [&](auto rule) {
        const auto& [name, depth, level] = rule;
        if (name.size() == 0) {
          cfg.levels.levels_by_depth.resize(depth+1, level);
        } else {
          const auto old_len = cfg.levels.name_data.size();
          std::copy(name.begin(), name.end(), std::back_inserter(cfg.levels.name_data));
          cfg.levels.levels_by_name.emplace_back(
            std::string_view{cfg.levels.name_data.data() + old_len, name.size()}, level);
        }
      });

      applyConfig(std::move(cfg));
    }
  }
} // namespace np
