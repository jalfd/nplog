#include <nplog/Log.hpp>

#include <algorithm>
#include <functional>
#include <iostream>
#include <mutex>

namespace {
    std::mutex sink_mtx;
    std::function<void(int, std::string_view msg)> log_sink;
    const std::function<void(int, std::string_view)> stderr_log_sink
      = [](int, std::string_view buffer) {
          std::string str(buffer.begin(), buffer.end());
          std::cerr << str << '\n';
        };
}
namespace np {
  std::function<void(int, std::string_view)> getStdErrSink() { return stderr_log_sink; }

  void Log::setSink(std::function<void(int, std::string_view msg)> sink) {
      std::lock_guard lock(sink_mtx);
      log_sink = sink;
  }

  bool Log::testMessage(int level) { return level < 3; } // TODO: implement this properly
  int Log::argThreshold() const { return 3; } // TODO: implement this properly

  Log::buffer_type Log::acquireBuffer() {
    std::lock_guard lock(buffer_mutex);
    if (buffers.empty()) { buffers.emplace_back(); }
    const auto buf = buffers.back();
    buffers.pop_back();
    return buf;
  }

  void Log::submitMessage(int level, const buffer_type& buffer) {
      if (log_sink) {
          log_sink(level, std::string_view{&buffer[0], buffer.size()});
      }
  }

  void Log::releaseBuffer(buffer_type buf) {
    buf.clear();
    std::lock_guard lock(buffer_mutex);
    buffers.push_back(buf);
  }
} // namespace np
