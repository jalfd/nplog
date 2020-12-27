#ifndef NP_TO_STRING_HELPER_HPP
#define NP_TO_STRING_HELPER_HPP

#include <algorithm>
#include <string_view>

namespace np::log {
  // will return a pointer to the first non-zero digit
  template <typename T>
  inline char* fixed_unsigned_to_decimal(T number, char* buffer, size_t len) noexcept {
    char* ptr = buffer + len - 1;
    if (number == 0) {
      *ptr-- = '0';
    } else {
      while (number != 0) {
        *ptr-- = static_cast<char>('0' + static_cast<char>(number % 10));
        number /= 10;
      }
    }
    return ptr + 1;
  }

  // will return a pointer past the end of the number
  template <typename T>
  inline char* unsigned_to_decimal(T number, char* buffer, size_t len) noexcept {
    const auto actual_begin = fixed_unsigned_to_decimal(number, buffer, len);
    const auto actual_len = buffer + len - actual_begin;
    std::copy(actual_begin, buffer + len, buffer);
    return buffer + actual_len;
  }

  template <typename T>
  auto to_unsigned(T number) noexcept {
    using UT = typename std::make_unsigned<T>::type;
    auto unum = static_cast<UT>(number);
    unum = 0 - unum;
    return unum;
  }
  /// Takes a view spanning the available buffer
  /// Returns a string_view spanning the written number
  /// Unused bytes at the end of the buffer
  template <typename T>
  inline std::string_view decimal_from(T number, char* first, char* last) noexcept {
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
  /// Unused bytes at the beginning of the buffer, initialized to pad
  inline void padded_decimal_from(unsigned int number, char* first, size_t width) noexcept {
    const auto start = fixed_unsigned_to_decimal(number, first, width);
    std::fill(first, start, '0');
  }
} // namespace np::log

#endif
