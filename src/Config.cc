#include <nplog/Config.hpp>
#include <algorithm>
#include <atomic>
#include <functional>
#include <map>
#include <numeric>
#include <string>
#include <vector>
#include "ConfigImpl.hpp"

namespace np {
  namespace {
    Levels merge(Levels lhs, Levels rhs) {
      return {std::min(lhs.message, rhs.message), std::min(lhs.param, rhs.param)};
    }
  } // namespace
  struct LogConfig {
    LogConfig() = default;
    LogConfig(const LogConfig& other) = delete;

    std::vector<std::pair<std::string_view, Levels>> levels_by_name;
    std::vector<Levels> levels_by_depth;
    std::vector<char> namedata;

    Levels default_levels;
  };

  std::atomic<unsigned> config_timestamp;
  std::shared_ptr<const LogConfig> config_ptr = std::make_shared<const LogConfig>();

  void Config::apply() const {
    auto new_ptr = std::make_shared<LogConfig>();
    LogConfig& cfg = *new_ptr;

    cfg.default_levels = default_levels;

    for (const auto [d, l] : levels_by_depth) {
      while (cfg.levels_by_depth.size() <= d) {
        cfg.levels_by_depth.push_back(l);
      }
    }

    const auto& lbn = levels_by_name;
    size_t name_len = std::accumulate(
      lbn.begin(), lbn.end(), size_t(), [](auto acc, auto p) { return acc + p.first.size(); });
    cfg.namedata.reserve(name_len);

    for (const auto [n, l] : levels_by_name) {
      const auto offset = cfg.namedata.size();
      std::copy(n.begin(), n.end(), std::back_inserter(cfg.namedata));
      const auto length = cfg.namedata.size() - offset;
      cfg.levels_by_name.emplace_back(std::string_view(&cfg.namedata[offset], length), l);
    }

    config_ptr = new_ptr;
    std::atomic_fetch_add_explicit(&config_timestamp, 1u, std::memory_order_release);
  }

  bool isCurrent(unsigned v) {
    return v == std::atomic_load_explicit(&config_timestamp, std::memory_order_acquire);
  }

  std::pair<Levels, unsigned> getLevels(std::string_view n, unsigned d) {
    auto version = std::atomic_load_explicit(&config_timestamp, std::memory_order_relaxed);

    auto ptr = config_ptr;
    auto& cfg = *ptr;

    auto lvls = cfg.default_levels;

    if (d < cfg.levels_by_depth.size()) { lvls = merge(lvls, cfg.levels_by_depth[d]); }

    const auto fun = [](const auto lhs, const auto rhs) { return lhs.first < rhs.first; };

    if (!n.empty()) {
      const auto& lbn = cfg.levels_by_name;
      const auto it = std::lower_bound(lbn.begin(), lbn.end(), std::pair{n, Levels{}}, fun);

      if (it != lbn.end() && it->first == n) { lvls = merge(lvls, it->second); }
    }

    return {lvls, version};
  }
} // namespace np
