#ifndef NP_STRINGINTERNER_HPP
#define NP_STRINGINTERNER_HPP

#include <nplog/serializer.hpp>
#include <shared_mutex>
#include <unordered_map>
#include <vector>
#include <iterator>
#include "messagebuffer.hpp"

namespace np::log {
  template <size_t default_buffer_size>
  struct StringInterner {
    StringInterner() {
      intern_buffers.resize(1);
      intern_buffers[0].reserve(default_buffer_size);
    }
    // this needs to hold a stable list of strings. Ideally, mushed together
    // and ideally we can generate string views from them

    std::string_view intern(const char* string) {
      std::string_view view_of_string;
      {
        std::shared_lock lock(mutex);
        const auto ptr_lookup_it = ptr_index.find(string);
        if (ptr_lookup_it != ptr_index.end()) { return ptr_lookup_it->second; }
        view_of_string = string;
        const auto str_lookup_it = string_index.find(view_of_string);
        if (str_lookup_it != string_index.end()) { return str_lookup_it->second; }
      }
      {
        std::unique_lock lock(mutex);
        // first, jsonify the string
        auto buffer = acquireBuffer();
        ValueSerializer vs(buffer);
        vs.write(view_of_string);
        // if the input string is too big to fit in the current buffer, allocate a new one and
        // ensure its size is sufficient
        if (buffer->contents().size() + 1 > default_buffer_size - intern_buffers.back().size()) {
          intern_buffers.resize(intern_buffers.size() + 1);
          intern_buffers.back().resize(
            std::max(default_buffer_size, buffer->contents().size() + 1));
        }
        const auto sv = buffer->contents();
        size_t start_offset = intern_buffers.back().size();
        std::copy(sv.begin(), sv.end(), std::back_inserter(intern_buffers.back()));
        intern_buffers.back().push_back('\0');
        releaseBuffer(buffer);

        std::string_view interned_sv(
          &intern_buffers.back()[start_offset], intern_buffers.back().size() - start_offset - 1);
        ptr_index.insert({string, interned_sv});
        string_index.insert({view_of_string, interned_sv});
        return interned_sv;
      }
    }

    std::vector<std::vector<char>> intern_buffers;
    std::unordered_map<const char*, std::string_view> ptr_index;
    std::unordered_map<std::string_view, std::string_view> string_index;
    std::shared_mutex mutex;
  };

  extern StringInterner<1024 * 1024> global_interner;
} // namespace np::log

#endif