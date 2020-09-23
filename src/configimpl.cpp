#include "configimpl.hpp"
#include <atomic>

namespace np::log {
  std::shared_mutex config_mutex;
  std::atomic<unsigned> config_timestamp = 1;

  namespace {
    const Config::Sink stderr_log_sink = [](MessageInfo msg) {
      fwrite(msg.message.data(), sizeof(char), msg.message.size(), stderr);
    };

    auto config_ptr = std::make_shared<LogConfig>();
  } // namespace

  void applyConfig(LogConfig cfg) {
    std::lock_guard lock(config_mutex);
    const auto new_config = std::make_shared<LogConfig>(std::move(cfg));
    if (!new_config->sink) { new_config->sink = stderr_log_sink; }
    std::atomic_store_explicit(&config_ptr, new_config, std::memory_order_release);
    std::atomic_fetch_add_explicit(&config_timestamp, 1u, std::memory_order_relaxed);
  }

  bool isCurrent(unsigned v) {
    return v == std::atomic_load_explicit(&config_timestamp, std::memory_order_acquire);
  }

  LevelsResult getLevels(std::string_view n, unsigned d) {
    auto version = std::atomic_load_explicit(&config_timestamp, std::memory_order_relaxed);

    std::shared_lock<std::shared_mutex> lock(config_mutex);

    const std::shared_ptr<LogConfig> current_config
      = std::atomic_load_explicit(&config_ptr, std::memory_order_acquire);
    LevelSpec lvls = current_config->levels.default_level;

    const auto fun = [](const auto lhs, const auto rhs) { return lhs.first < rhs.first; };

    if (!n.empty()) {
      const auto& lbn = current_config->levels.levels_by_name;
      const auto it = std::lower_bound(lbn.begin(), lbn.end(), std::pair{n, LevelSpec{}}, fun);

      if (it != lbn.end() && it->first == n) { lvls = merge(lvls, it->second); }
    }

    auto levels_by_name = lvls;

    if (d < current_config->levels.levels_by_depth.size()) {
      lvls = merge(lvls, current_config->levels.levels_by_depth[d]);
    }

    return {version, lvls, levels_by_name, current_config->levels.sensitive};
  }

  std::atomic<Config::Fields> cached_enabled_fields = Config::Fields();
  std::atomic<unsigned> fields_version = 0;

  Config::Fields enabledFields() {
    if (isCurrent(fields_version)) {
      return std::atomic_load_explicit(&cached_enabled_fields, std::memory_order_acquire);
    }
    // the fields value we're about to retrieve will have at least this version
    const auto version = std::atomic_load_explicit(&config_timestamp, std::memory_order_relaxed);

    const auto current_fields
      = std::atomic_load_explicit(&config_ptr, std::memory_order_acquire)->fields;

    std::atomic_store_explicit(&cached_enabled_fields, current_fields, std::memory_order_release);

    // Update the cached version after updating the cache. This ensures we'll never think we're up
    // to date when we're not
    std::atomic_store_explicit(&fields_version, version, std::memory_order_release);
    // finally, return the value we picked
    return current_fields;
  }

  // Make this atomic... somehow.
  // No, this must actually lock. We need to serialize sink calls
  void sendToSink(level_type level, std::string_view buffer) {
    const std::shared_ptr<LogConfig> current_config
      = std::atomic_load_explicit(&config_ptr, std::memory_order_acquire);
    current_config->sink(MessageInfo{level, buffer});
  }
} // namespace np::log
