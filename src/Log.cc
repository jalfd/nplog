#include <nplog/Log.hpp>
#include "ConfigImpl.hpp"

#include <algorithm>
#include <atomic>
#include <functional>
#include <cstdio>
#include <mutex>

namespace np {
  namespace {
    struct LogState {
      std::mutex buffer_mutex;
      std::vector<Log::buffer_type> buffers;
    };

    static LogState state;

    const std::function<void(int, std::string_view)> stderr_log_sink
      = [](int, std::string_view buffer) {
          std::fprintf(stderr, "%s\n", &buffer[0]);
        };
  } // namespace

  std::function<void(int, std::string_view)> getStdErrSink() { return stderr_log_sink; }

  Log::Log(Log* parent, const char* name)
    : parent(parent), name(name), name_len(name ? strlen(name) : 0)
    , depth(parent ? parent->depth + 1 : 0)
  {
      refreshLevels(0);
  }

  Log::Log(const char* name) : Log(nullptr, name) {}

  Levels Log::refreshLevels(unsigned version, bool exclude_depth) {
    if (!isCurrent(version)) {
      auto result = getLevels(std::string_view(name, name_len), depth);
      levels_by_name_only = result.levels_by_name_only;
      effective_levels = result.effective_levels;

      if (parent) {
        auto parent_levels = parent->refreshLevels(result.version, true);
        effective_levels = merge(effective_levels, parent_levels);
        levels_by_name_only = merge(levels_by_name_only, parent_levels);
      }
      this->version = result.version;
    }
    return exclude_depth ? levels_by_name_only : effective_levels;
  }

  Log::buffer_type Log::acquireBuffer() {
    std::lock_guard lock(state.buffer_mutex);
    if (state.buffers.empty()) { state.buffers.emplace_back(); }
    const auto buf = std::move(state.buffers.back());
    state.buffers.pop_back();
    return buf;
  }

  void Log::submitMessage(int level, buffer_type& buffer) {
    buffer.push_back('\0');
    ::np::sendToSink(level, std::string_view{&buffer[0], buffer.size() - 1});
  }

  void Log::releaseBuffer(buffer_type&& buf) {
    buf.clear();
    std::lock_guard lock(state.buffer_mutex);
    state.buffers.push_back(std::move(buf));
  }
} // namespace np
