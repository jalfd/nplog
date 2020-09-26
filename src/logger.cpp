#include <nplog/logger.hpp>
#include "configimpl.hpp"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <functional>
#include <mutex>
#include <string>

namespace np::log {
  namespace {
    struct LogState {
      std::mutex buffer_mutex;
      std::vector<Logger::buffer_type> buffers;
    };

    static LogState state;

  } // namespace

  LoggerParams::LoggerParams(LoggerParams* parent, std::initializer_list<LogParam> params) {
    Serializer s(&data);
    std::vector<const LogParam*> param_ptrs; // FIXME: should reserve
    std::transform(
      params.begin(), params.end(), std::back_inserter(param_ptrs), [](const LogParam& p) { return &p; });
    std::sort(param_ptrs.begin(), param_ptrs.end(), [](const LogParam* lhs, const LogParam* rhs) {
      return strcmp(lhs->name, rhs->name) < 0;
    });
    for (const auto& param_ptr : param_ptrs) {
      const auto start = data.messageSize();
      const auto name_end = param_ptr->func(s, data);
      const auto end = data.messageSize();
      offsets.emplace_back(start, name_end, end);
    }

    // Merge with parent's params
    if (parent) {
      auto parent_it = parent->offsets.begin();
      auto self_it = offsets.begin();

      // ok, so we need a new buffer
      LoggerParams dest; // should reserve member size

      const auto copy_to_dest
        = [&dest](const LoggerParams& src, size_t first, size_t name_last, size_t last) {
            const auto new_first = dest.data.messageSize();
            const auto span
              = src.data.contents().substr(first, last); // the span of characters to copy
            auto* insert_at = dest.data.insertAt(last - first);
            span.copy(insert_at, last - first, first);

            dest.offsets.emplace_back(
              new_first, new_first + (name_last - first), new_first + (last - first));
          };

      while (parent_it != parent->offsets.end() && self_it != offsets.end()) {
        const auto parent_name
          = std::string_view(parent->data.contents().substr(std::get<0>(*parent_it),
            std::get<1>(*parent_it) - std::get<0>(*parent_it))); // FIXME: this is a very long line
        const auto self_name = std::string_view(data.contents().substr(
          std::get<0>(*self_it), std::get<1>(*self_it) - std::get<0>(*self_it)));

        auto* current = parent_name < self_name ? parent : this;
        auto& current_it = parent_name < self_name ? parent_it : self_it;
        copy_to_dest(
          *current, std::get<0>(*current_it), std::get<1>(*current_it), std::get<2>(*current_it));
        ++current_it;
      }

      // whichever range has stuff left, just copy it in
      for (; parent_it != parent->offsets.end(); ++parent_it) {
        copy_to_dest(
          *parent, std::get<0>(*parent_it), std::get<1>(*parent_it), std::get<2>(*parent_it));
      }
      for (; self_it != offsets.end(); ++self_it) {
        copy_to_dest(*this, std::get<0>(*self_it), std::get<1>(*self_it), std::get<2>(*self_it));
      }
      *this = std::move(dest);
    }
  }

  Logger::Logger(Logger* parent, const char* name)
    : parent(parent)
    , name_ptr(name)
    , name_len(name_ptr ? strlen(name_ptr) : 0)
    , depth(parent ? parent->depth + 1 : 0) {
    refreshLevels(0);
  }

  Logger::Logger(const char* name) : Logger(nullptr, name) {}

  Logger::~Logger() {
      // Delete, unless we're pointing at our parent's params
    if (!(parent && parent->logger_params == logger_params)) { delete logger_params; }
  }

  LevelSpec Logger::refreshLevels(unsigned version_, bool exclude_depth) {
    if (!isCurrent(version_)) {
      auto result = getLevels(std::string_view(name_ptr, name_len), depth);
      levels_by_name_only = result.levels_by_name_only;
      effective_levels = result.effective_levels;

      if (parent) {
        auto parent_levels = parent->refreshLevels(result.version, true);
        effective_levels = merge(effective_levels, parent_levels);
        levels_by_name_only = merge(levels_by_name_only, parent_levels);
      }
      version = result.version;
    }
    return exclude_depth ? levels_by_name_only : effective_levels;
  }

  Logger::buffer_type Logger::acquireBuffer() {
    std::lock_guard lock(state.buffer_mutex);
    if (state.buffers.empty()) { state.buffers.emplace_back(); }
      auto buf = std::move(state.buffers.back());
      state.buffers.pop_back();
      return buf;
  }

  void Logger::submitMessage(level_type level, buffer_type& buffer) {
    ::np::log::sendToSink(level, buffer.contents());
  }

  void Logger::releaseBuffer(buffer_type&& buf) {
    buf.clear();
    std::lock_guard lock(state.buffer_mutex);
    state.buffers.push_back(std::move(buf));
  }
} // namespace np
