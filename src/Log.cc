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

      std::mutex sink_mtx;
      std::function<void(int, std::string_view msg)> log_sink;
    };

    static LogState state;

    const std::function<void(int, std::string_view)> stderr_log_sink
      = [](int, std::string_view buffer) {
          std::fprintf(stderr, "%s\n", &buffer[0]);
        };
  } // namespace

  std::function<void(int, std::string_view)> getStdErrSink() { return stderr_log_sink; }

  Log::Log(Log* parent, const char* name)
    : name(name), name_len(name ? strlen(name) : 0)
    , depth(parent ? parent->depth + 1 : 0)
  {
      auto lv = getLevels(std::string_view(name, name_len), depth);
      levels = lv.first;
      version = lv.second;
  }

  Log::Log(const char* name) : Log(nullptr, name) {}

  void Log::setSink(std::function<void(int, std::string_view msg)> sink) {
    std::lock_guard lock(state.sink_mtx);
    state.log_sink = sink;
  }

  Levels Log::refreshLevels()
  {
    if (!isCurrent(version)) {
      auto lv = getLevels(std::string_view(name, name_len), depth);
      levels = lv.first;
      version = lv.second;
    }
    return levels;
  }

  Log::buffer_type Log::acquireBuffer() {
    std::lock_guard lock(state.buffer_mutex);
    if (state.buffers.empty()) { state.buffers.emplace_back(); }
    const auto buf = std::move(state.buffers.back());
    state.buffers.pop_back();
    return buf;
  }

  void Log::submitMessage(int level, buffer_type& buffer) {
    std::lock_guard lock(state.sink_mtx);
    if (state.log_sink) {
      buffer.push_back('\0');
      state.log_sink(level, std::string_view{&buffer[0], buffer.size() - 1});
    }
  }

  void Log::releaseBuffer(buffer_type&& buf) {
    buf.clear();
    std::lock_guard lock(state.buffer_mutex);
    state.buffers.push_back(std::move(buf));
  }
} // namespace np
