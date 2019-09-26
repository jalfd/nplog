#ifndef NPLOG_MESSAGEBUFFER_HPP
#define NPLOG_MESSAGEBUFFER_HPP

#include <algorithm>
#include <string_view>
#include <cstdlib>

namespace np::log {
    struct MessageBuffer {
        MessageBuffer() noexcept = default;
        ~MessageBuffer() noexcept { delete[] buf_begin; }
        MessageBuffer(MessageBuffer&& other) noexcept : buf_begin(other.buf_begin), buf_end(other.buf_end), msg_end(other.msg_end) {
            other.buf_begin = nullptr;
            other.buf_end = nullptr;
            other.msg_end = nullptr;
        }
        MessageBuffer& operator=(MessageBuffer&& other) noexcept {
          delete[] buf_begin;
          buf_begin = other.buf_begin;
          buf_end = other.buf_end;
          msg_end = other.msg_end;
          other.buf_begin = nullptr;
          other.buf_end = nullptr;
          other.msg_end = nullptr;
          return *this;
        }

        void reserve(size_t requested_size) noexcept {
          // always make sure there's room for a null byte
          // beyond the requested size
          const auto minimum_size = requested_size + 1;
          if (minimum_size <= buf_end - buf_begin) { return; }
          // never grow by less than a factor 2x
          const auto new_size = std::max(bufferSize() * 2, minimum_size);
          grow(new_size);
        }
        void reserveAdditional(size_t additional) noexcept{
            reserve(additional + messageSize());
        }
        void append(char c) noexcept {
          reserveAdditional(1);
          *msg_end++ = c;
        }
        char* insertAt(size_t insert_length) noexcept { 
          reserveAdditional(insert_length);
            char* insert_point = msg_end;
            msg_end += insert_length;
            return insert_point;
        }

        void shrinkTo(size_t new_length) noexcept
        {
            if (new_length > bufferSize()) { std::abort(); }
            msg_end = buf_begin + new_length;
        }
        std::string_view contents() const noexcept {
          *msg_end = '\0';
          return std::string_view{buf_begin, messageSize()};
        };

        void clear() noexcept
        {
            msg_end = buf_begin;
        }

        size_t bufferSize() const noexcept {
          // don't tell user about the last byte of the buffer, reserved for null termination
          return buf_begin ? (buf_end - buf_begin) - 1 : 0;
        }
        size_t messageSize() const noexcept { return msg_end - buf_begin; }

      private:
        void grow(size_t new_size) noexcept {
          char* new_buf = new char[new_size];
          msg_end = std::copy(buf_begin, msg_end, new_buf);
          buf_end = new_buf + new_size;
          delete[] buf_begin;
          buf_begin = new_buf;
        }

        char* buf_begin = nullptr;
        char* buf_end = nullptr;
        char* msg_end = nullptr;
    };
} // namespace np

#endif
