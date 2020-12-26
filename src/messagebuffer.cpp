#include "messagebuffer.hpp"
#include <mutex>
#include <vector>

namespace np::log {
  namespace {
    struct LogState {
      std::mutex buffer_mutex;
      std::vector<MessageBuffer*> buffers;
    };

    static LogState state;
  } // namespace
  MessageBuffer* acquireBuffer() noexcept {
    std::lock_guard lock(state.buffer_mutex);
    if (state.buffers.empty()) { state.buffers.emplace_back(new MessageBuffer()); }
    auto buf = state.buffers.back();
    state.buffers.pop_back();
    return buf;
  }

  void releaseBuffer(MessageBuffer* buf) noexcept {
    buf->clear();
    std::lock_guard lock(state.buffer_mutex);
    state.buffers.push_back(buf);
  }
} // namespace np::log
