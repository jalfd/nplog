#include "configimpl.hpp"
#include <atomic>
#include <memory>
#include <mutex>

namespace np::log {
  namespace {
    std::mutex sink_mutex;
    std::atomic<unsigned> config_timestamp = 1;
    const Config::Sink stderr_log_sink = [](MessageInfo msg) {
      fwrite(msg.message.data(), sizeof(char), msg.message.size(), stderr);
    };

    auto config = std::make_shared<LogConfig>();

    std::shared_ptr<LogConfig> getConfig(unsigned known_global_version) {
      thread_local unsigned cached_version = 0;
      thread_local std::shared_ptr<LogConfig> cached_ptr;

      if (!isCurrent(cached_version, known_global_version)) {
        cached_ptr = std::atomic_load_explicit(&config, std::memory_order_acquire);
        cached_version = currentVersion();
      }

      return cached_ptr;
    }
  } // namespace

  unsigned currentVersion() {
    return std::atomic_load_explicit(&config_timestamp, std::memory_order_acquire);
  }

  void applyConfig(LogConfig cfg) {
    const auto new_cfg = std::make_shared<LogConfig>(std::move(cfg));
    if (!cfg.sink) { cfg.sink = stderr_log_sink; }
    std::atomic_store_explicit(&config, new_cfg, std::memory_order_release);
    std::atomic_fetch_add_explicit(&config_timestamp, 1u, std::memory_order_release);
  }

  LevelsResult getLevels(std::string_view n, unsigned d) {
    auto version = currentVersion();

    const auto cfg_ptr = getConfig(version);
    const auto& cfg = *cfg_ptr;

    LevelSpec lvls = cfg.levels.default_level;

    const auto fun = [](const auto lhs, const auto rhs) { return lhs.first < rhs.first; };

    if (!n.empty()) {
      const auto& lbn = cfg.levels.levels_by_name;
      const auto it = std::lower_bound(lbn.begin(), lbn.end(), std::pair{n, LevelSpec{}}, fun);

      if (it != lbn.end() && it->first == n) { lvls = merge(lvls, it->second); }
    }

    auto levels_by_name = lvls;

    if (d < cfg.levels.levels_by_depth.size()) {
      lvls = merge(lvls, cfg.levels.levels_by_depth[d]);
    }

    return {version, lvls, levels_by_name};
  }

  Config::Fields enabledFields(unsigned global_version) {
    thread_local unsigned cached_version = 0;
    thread_local Config::Fields cached_fields;

    if (!isCurrent(cached_version, global_version)) {
      cached_fields = getConfig(global_version)->fields;
      cached_version = currentVersion();
    }
    return cached_fields;
  }

  void sendToSink(level_type level, std::string_view buffer, unsigned global_version) {
    std::lock_guard<std::mutex> lock(sink_mutex);
    getConfig(global_version)->sink(MessageInfo{level, buffer});
  }
} // namespace np::log
