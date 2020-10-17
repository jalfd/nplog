#include <nplog/messagebuffer.hpp>
#include <mutex>
#include <vector>

namespace np::log {
  namespace {
    struct LogState {
      std::mutex buffer_mutex;
      std::vector<MessageBuffer> buffers;
    };

    static LogState state;
  } // namespace
  MessageBuffer acquireBuffer() {
    std::lock_guard lock(state.buffer_mutex);
    if (state.buffers.empty()) { state.buffers.emplace_back(); }
    auto buf = std::move(state.buffers.back());
    state.buffers.pop_back();
    return buf;
  }

  void releaseBuffer(MessageBuffer&& buf) {
    buf.clear();
    std::lock_guard lock(state.buffer_mutex);
    state.buffers.push_back(std::move(buf));
  }
} // namespace np::log
