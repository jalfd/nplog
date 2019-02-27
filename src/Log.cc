#include <nplog/Log.hpp>

#include <algorithm>
#include <functional>
#include <iostream>
#include <mutex>

namespace np {
  namespace {
    struct LogState {
      std::mutex buffer_mutex;
      std::vector<Log::buffer_type> buffers;

      std::mutex sink_mtx;
      std::function<void(int, std::string_view msg)> log_sink;

      std::atomic<int> message_level = 0;
      std::atomic<int> param_level = 0;
    };

    static LogState state;

    const std::function<void(int, std::string_view)> stderr_log_sink
      = [](int, std::string_view buffer) {
          std::string str(buffer.begin(), buffer.end());
          std::cerr << str << '\n';
        };
  } // namespace

  std::function<void(int, std::string_view)> getStdErrSink() { return stderr_log_sink; }

  Log::Log()
    : message_level(state.message_level.load(std::memory_order_relaxed))
    , param_level(state.param_level.load(std::memory_order_relaxed)) {}

  void Log::setSink(std::function<void(int, std::string_view msg)> sink) {
    std::lock_guard lock(state.sink_mtx);
    state.log_sink = sink;
  }
  void Log::setMessageLevel(int level) {
    state.message_level.store(level, std::memory_order_relaxed);
  }
  void Log::setParamLevel(int level){
    state.param_level.store(level, std::memory_order_relaxed);
  }

  bool Log::suppressMessage(int level) const {
    return level > state.message_level.load(std::memory_order_relaxed);
  }

  int Log::paramLevel() const { 
    return state.param_level.load(std::memory_order_relaxed);
  }

  Log::buffer_type Log::acquireBuffer() {
    std::lock_guard lock(state.buffer_mutex);
    if (state.buffers.empty()) { state.buffers.emplace_back(); }
    const auto buf = state.buffers.back();
    state.buffers.pop_back();
    return buf;
  }

  void Log::submitMessage(int level, const buffer_type& buffer) {
    std::lock_guard lock(state.sink_mtx);
    if (state.log_sink) { state.log_sink(level, std::string_view{&buffer[0], buffer.size()}); }
  }

  void Log::releaseBuffer(buffer_type buf) {
    buf.clear();
    std::lock_guard lock(state.buffer_mutex);
    state.buffers.push_back(buf);
  }
} // namespace np
