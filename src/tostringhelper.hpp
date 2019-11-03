#ifndef NP_TO_STRING_HELPER_HPP
#define NP_TO_STRING_HELPER_HPP

#include <algorithm>
#include <string_view>

namespace np::log {
  // will return a pointer to the first non-zero digit
  template <typename T>
  inline char* fixed_unsigned_to_decimal(T number, char* buffer, size_t len) {
    char* ptr = buffer + len - 1;
    if (number == 0) {
      *ptr-- = '0';
    } else {
      while (number != 0) {
        *ptr-- = static_cast<char>('0' + number % 10);
        number /= 10;
      }
    }
    return ptr + 1;
  }

  // will return a pointer to the first non-zero digit
  // prior digits will be set to pad
  template <typename T>
  inline char* pad_unsigned_to_decimal(T number, char* buffer, size_t len, char pad) {
    const auto actual_begin = fixed_unsigned_to_decimal(number, buffer, len);
    std::fill(buffer, actual_begin, pad);
    return actual_begin;
  }

  // will return a pointer past the end of the number
  template <typename T>
  inline char* unsigned_to_decimal(T number, char* buffer, size_t len) {
    const auto actual_begin = fixed_unsigned_to_decimal(number, buffer, len);
    const auto actual_len = buffer + len - actual_begin;
    std::copy(actual_begin, buffer + len, buffer);
    return buffer + actual_len;
  }

  template <typename T>
  auto to_unsigned(T number) {
    using UT = typename std::make_unsigned<T>::type;
    auto unum = static_cast<UT>(number);
    unum = 0 - unum;
    return unum;
  }
  /// Takes a view spanning the available buffer
  /// Returns a string_view spanning the written number
  /// Unused bytes at the end of the buffer
  template <typename T>
  inline std::string_view decimal_from(T number, char* first, char* last) {
    if constexpr (std::is_signed_v<T>) {
      if (number < 0) {
        auto n = to_unsigned(number);
        char* end = unsigned_to_decimal(n, first + 1, to_size_t_checked(last - first - 1));
        *first = '-';
        return std::string_view(first, to_size_t_checked(end - first));
      }
    }
    const auto end = unsigned_to_decimal(number, first, to_size_t_checked(last - first));
    return std::string_view(first, to_size_t_checked(end - first));
  }

  /// Takes a view spanning the available buffer
  /// Returns a string_view spanning the written number
  /// Unused bytes at the beginning of the buffer
  template <typename T>
  inline std::string_view fixed_decimal_from(T number, char* first, char* last) {
    if constexpr (std::is_signed_v<T>) {
      if (number < 0) {
        auto n = to_unsigned(number);
        char* start = fixed_unsigned_to_decimal(n, first + 1, to_size_t_checked(last - first - 1));
        *--start = '-';
        return std::string_view(start, to_size_t_checked(last - start));
      }
    }
    const auto start = fixed_unsigned_to_decimal(number, first, to_size_t_checked(last - first));
    return std::string_view(start, to_size_t_checked(last - start));
  }

  /// Takes a view spanning the available buffer
  /// Returns a string_view spanning the written number
  /// Unused bytes at the beginning of the buffer, initialized to pad
  template <typename T>
  inline std::string_view padded_decimal_from(T number, char* first, char* last, char pad) {
    if constexpr (std::is_signed_v<T>) {
      if (number < 0) {
        auto n = to_unsigned(number);
        char* start = fixed_unsigned_to_decimal(n, first + 1, to_size_t_checked(last - first - 1));
        *--start = '-';
        std::fill(first, start, pad);
        return std::string_view(start, to_size_t_checked(last - start));
      }
    }
    const auto start = fixed_unsigned_to_decimal(number, first, to_size_t_checked(last - first));
    std::fill(first, start, pad);
    return std::string_view(start, to_size_t_checked(last - start));
  }
} // namespace np

#endif
