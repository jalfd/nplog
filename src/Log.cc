#include <nplog/Log.hpp>
#include "ConfigImpl.hpp"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <functional>
#include <mutex>
#include <string>

namespace np {
  namespace {
    struct LogState {
      std::mutex buffer_mutex;
      std::vector<Log::buffer_type> buffers;
    };

    static LogState state;

  } // namespace

  Log::Log(Log* parent, const char* name)
    : parent(parent)
    , name_ptr(name)
    , name_len(name_ptr ? strlen(name_ptr) : 0)
    , depth(parent ? parent->depth + 1 : 0) {
    refreshLevels(0);
  }

  Log::Log(const char* name) : Log(nullptr, name) {}

  LevelSpec Log::refreshLevels(unsigned version, bool exclude_depth) {
    if (!isCurrent(version)) {
      auto result = getLevels(std::string_view(name_ptr, name_len), depth);
      levels_by_name_only = result.levels_by_name_only;
      effective_levels = result.effective_levels;
      sensitive = result.sensitive;

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
      auto buf = std::move(state.buffers.back());
      state.buffers.pop_back();
      return buf;
  }

  void Log::submitMessage(level_type level, buffer_type& buffer) {
    ::np::sendToSink(level, buffer.contents());
  }

  void Log::releaseBuffer(buffer_type&& buf) {
    buf.clear();
    std::lock_guard lock(state.buffer_mutex);
    state.buffers.push_back(std::move(buf));
  }
} // namespace np
