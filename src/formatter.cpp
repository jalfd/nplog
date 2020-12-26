#include <nplog/formatter.hpp>
#include <nplog/serializer.hpp>
#include <string>
namespace np::log {
  void format(bool val, ValueSerializer& srl) noexcept { srl.write(val); }

  void format(short val, ValueSerializer& srl) noexcept { srl.write(static_cast<int>(val)); }

  void format(unsigned short val, ValueSerializer& srl) noexcept {
    srl.write(static_cast<unsigned int>(val));
  }

  void format(int val, ValueSerializer& srl) noexcept { srl.write(val); }

  void format(unsigned int val, ValueSerializer& srl) noexcept { srl.write(val); }

  void format(long val, ValueSerializer& srl) noexcept {
    if constexpr (sizeof(long) == sizeof(int)) {
      srl.write(static_cast<int>(val));
    } else {
      srl.write(static_cast<long long>(val));
    }
  }

  void format(unsigned long val, ValueSerializer& srl) noexcept {
    if constexpr (sizeof(unsigned long) == sizeof(unsigned int)) {
      srl.write(static_cast<unsigned int>(val));
    } else {
      srl.write(static_cast<unsigned long long>(val));
    }
  }

  void format(long long val, ValueSerializer& srl) noexcept { srl.write(val); }

  void format(unsigned long long val, ValueSerializer& srl) noexcept { srl.write(val); }

  void format(float val, ValueSerializer& srl) noexcept { srl.write(val); }

  void format(double val, ValueSerializer& srl) noexcept { srl.write(val); }

  void format(long double val, ValueSerializer& srl) noexcept { srl.write(val); }

  void format(std::string_view val, ValueSerializer& srl) noexcept { srl.write(val); }

  void format(const std::string& val, ValueSerializer& srl) noexcept { srl.write(std::string_view(val)); }
} // namespace np::log
