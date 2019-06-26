#include <nplog/Log.hpp>

#include <algorithm>
#include <cstdio>
#include <functional>
#include <mutex>
#include <string>

namespace {
  std::mutex sink_mtx;
  std::function<void(int, std::string_view msg)> log_sink;
  const std::function<void(int, std::string_view)> stderr_log_sink
    = [](int, std::string_view buffer) { std::fprintf(stderr, "%s\n", &buffer[0]); };
} // namespace
namespace np {
  std::function<void(int, std::string_view)> getStdErrSink() { return stderr_log_sink; }

  void Log::setSink(std::function<void(int, std::string_view msg)> sink) {
    std::lock_guard lock(sink_mtx);
    log_sink = sink;
  }

  bool Log::suppressMessage(int level) const {
    return level > 3;
  }

  int Log::paramLevel() const { return 3; }

  Log::buffer_type Log::acquireBuffer() {
    std::lock_guard lock(buffer_mutex);
    if (buffers.empty()) { buffers.emplace_back(); }
    const auto buf = std::move(buffers.back());
    buffers.pop_back();
    return buf;
  }

  void Log::submitMessage(int level, const buffer_type& buffer) {
    if (log_sink) { log_sink(level, std::string_view{&buffer[0], buffer.size()}); }
  }

  void Log::releaseBuffer(buffer_type&& buf) {
    buf.clear();
    std::lock_guard lock(buffer_mutex);
    buffers.push_back(std::move(buf));
  }

} // namespace np
