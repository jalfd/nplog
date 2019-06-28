#include <nplog/Log.hpp>

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <functional>
#include <mutex>
#include <string>

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
      = [](int, std::string_view buffer) { std::fprintf(stderr, "%s\n", &buffer[0]); };
  } // namespace

  std::function<void(int, std::string_view)> getStdErrSink() { return stderr_log_sink; }

  void Log::setSink(std::function<void(int, std::string_view msg)> sink) {
    std::lock_guard lock(state.sink_mtx);
    state.log_sink = sink;
  }

  bool Log::suppressMessage(int level) const {
    return level > 3;
  }

  int Log::paramLevel() const { return 3; }

  Log::buffer_type Log::acquireBuffer() {
    std::lock_guard lock(state.buffer_mutex);
    if (state.buffers.empty()) { state.buffers.emplace_back(); }
    const auto buf = std::move(state.buffers.back());
    state.buffers.pop_back();
    return buf;
  }

  void Log::submitMessage(int level, const buffer_type& buffer) {
    std::lock_guard lock(state.sink_mtx);
    if (state.log_sink) { state.log_sink(level, std::string_view{&buffer[0], buffer.size()}); }
  }

  void Log::releaseBuffer(buffer_type&& buf) {
    buf.clear();
    std::lock_guard lock(state.buffer_mutex);
    state.buffers.push_back(std::move(buf));
  }

} // namespace np
