#include <nplog/Formatter.hpp>
#include <nplog/Serializer.hpp>
#include <string>
namespace np {
  void format(bool val, ValueSerializer& srl) { srl.write(val); }

  void format(short val, ValueSerializer& srl) { srl.write(static_cast<int>(val)); }

  void format(unsigned short val, ValueSerializer& srl) {
    srl.write(static_cast<unsigned int>(val));
  }

  void format(int val, ValueSerializer& srl) { srl.write(val); }

  void format(unsigned int val, ValueSerializer& srl) { srl.write(val); }

  void format(long val, ValueSerializer& srl) {
    if constexpr (sizeof(long) == sizeof(int)) {
      srl.write(static_cast<int>(val));
    } else {
      srl.write(static_cast<long long>(val));
    }
  }

  void format(unsigned long val, ValueSerializer& srl) {
    if constexpr (sizeof(unsigned long) == sizeof(unsigned int)) {
      srl.write(static_cast<unsigned int>(val));
    } else {
      srl.write(static_cast<unsigned long long>(val));
    }
  }

  void format(long long val, ValueSerializer& srl) { srl.write(val); }

  void format(unsigned long long val, ValueSerializer& srl) { srl.write(val); }

  void format(float val, ValueSerializer& srl) { srl.write(val); }

  void format(double val, ValueSerializer& srl) { srl.write(val); }

  void format(long double val, ValueSerializer& srl) { srl.write(val); }

  void format(std::string_view val, ValueSerializer& srl) { srl.write(val); }

  void format(const std::string& val, ValueSerializer& srl) { srl.write(std::string_view(val)); }
} // namespace np
