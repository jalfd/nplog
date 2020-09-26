#include "configimpl.hpp"
#include <atomic>

namespace np::log {
  std::shared_mutex config_mutex;
  std::atomic<unsigned> config_timestamp = 1;

  namespace {
    const Config::Sink stderr_log_sink = [](MessageInfo msg) {
      fwrite(msg.message.data(), sizeof(char), msg.message.size(), stderr);
    };

    LogConfig config;
  } // namespace

  void applyConfig(LogConfig cfg) {
    std::lock_guard lock(config_mutex);
    config = std::move(cfg);
    if (!config.sink) { config.sink = stderr_log_sink; }
    std::atomic_fetch_add_explicit(&config_timestamp, 1u, std::memory_order_relaxed);
  }

  bool isCurrent(unsigned v) {
    return v == std::atomic_load_explicit(&config_timestamp, std::memory_order_acquire);
  }

  LevelsResult getLevels(std::string_view n, unsigned d) {
    auto version = std::atomic_load_explicit(&config_timestamp, std::memory_order_relaxed);

    std::shared_lock<std::shared_mutex> lock(config_mutex);

    LevelSpec lvls = config.levels.default_level;

    const auto fun = [](const auto lhs, const auto rhs) { return lhs.first < rhs.first; };

    if (!n.empty()) {
      const auto& lbn = config.levels.levels_by_name;
      const auto it = std::lower_bound(lbn.begin(), lbn.end(), std::pair{n, LevelSpec{}}, fun);

      if (it != lbn.end() && it->first == n) { lvls = merge(lvls, it->second); }
    }

    auto levels_by_name = lvls;

    if (d < config.levels.levels_by_depth.size()) {
      lvls = merge(lvls, config.levels.levels_by_depth[d]);
    }

    return {version, lvls, levels_by_name};
  }

  Config::Fields enabledFields() {
    std::shared_lock<std::shared_mutex> lock(config_mutex);
    return config.fields;
  }

  void sendToSink(level_type level, std::string_view buffer) {
    std::shared_lock<std::shared_mutex> lock(config_mutex); // TODO: is this where we serialize log messages from multiple threads?
    config.sink(MessageInfo{level, buffer});
  }
} // namespace np
