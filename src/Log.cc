#include <nplog/Log.hpp>

#include <algorithm>
#include <iostream>
#include <mutex>
#include <string>

namespace np {
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

  void Log::submitMessage(const buffer_type& buffer) {
    std::string str(buffer.begin(), buffer.end());
    std::cout << str << '\n';
  }

  void Log::releaseBuffer(buffer_type&& buf) {
    buf.clear();
    std::lock_guard lock(buffer_mutex);
    buffers.push_back(std::move(buf));
  }

} // namespace np
