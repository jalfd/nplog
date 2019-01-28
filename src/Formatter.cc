#include <nplog/Formatter.hpp>
#include <nplog/Serializer.hpp>
namespace np {
  void format(bool val, Serializer& srl) {
      srl.write(val);
  }

  void format(short val, Serializer& srl) {
      srl.write(static_cast<int>(val));
  }

  void format(unsigned short val, Serializer& srl) {
      srl.write(static_cast<unsigned int>(val));
  }

  void format(int val, Serializer& srl) {
      srl.write(val);
  }

  void format(unsigned int val, Serializer& srl) {
      srl.write(val);
  }

  void format(long val, Serializer& srl) {
      if constexpr(sizeof(long) == sizeof(int)) {
          srl.write(static_cast<int>(val));
      } else {
          srl.write(static_cast<long long>(val));
      }
  }

  void format(unsigned long val, Serializer& srl) {
      if constexpr(sizeof(unsigned long) == sizeof(unsigned int)) {
          srl.write(static_cast<unsigned int>(val));
      } else {
          srl.write(static_cast<unsigned long long>(val));
      }
  }

  void format(long long val, Serializer& srl) {
      srl.write(val);
  }

  void format(unsigned long long val, Serializer& srl) {
      srl.write(val);
  }

  void format(float val, Serializer& srl) {
      srl.write(val);
  }

  void format(double val, Serializer& srl) {
      srl.write(val);
  }

  void format(long double val, Serializer& srl) {
      srl.write(val);
  }

  void format(std::string_view val, Serializer& srl) {
      srl.write(val);
  }
} // namespace np
