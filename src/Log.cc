#include <nplog/Log.hpp>

#include <algorithm>
#include <iostream>
#include <mutex>

namespace np {
  bool Log::testMessage(int level) { return level < 3; } // TODO: implement this properly
  int Log::argThreshold() const { return 3; } // TODO: implement this properly

  Log::buffer_type Log::acquireBuffer() {
    std::lock_guard lock(buffer_mutex);
    if (buffers.empty()) { buffers.emplace_back(); }
    const auto buf = buffers.back();
    buffers.pop_back();
    return buf;
  }

  void Log::submitMessage(buffer_type buffer) {
    std::string str(buffer.begin(), buffer.end());
    std::cout << str << '\n';
  }

  void Log::releaseBuffer(buffer_type buf) {
    buf.clear();
    std::lock_guard lock(buffer_mutex);
    buffers.push_back(buf);
  }
} // namespace np
