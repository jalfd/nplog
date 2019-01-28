#include <nplog/Formatter.hpp>
#include <catch/catch.hpp>
#include "mocks.hpp"
#include <numeric>

template <typename In, typename Out>
void checkType()
{
  MockBuffer b;
  MockSerializer srl(&b);
  ops.clear();
  np::format(std::numeric_limits<In>::min(), srl);
  np::format(std::numeric_limits<In>::max(), srl);
  REQUIRE(ops.size() == 2);
  REQUIRE(std::get<0>(ops[0]) == "writeVal");
  REQUIRE(std::any_cast<Out>(std::get<1>(ops[0])) == std::numeric_limits<In>::min());
  REQUIRE(std::get<0>(ops[1]) == "writeVal");
  REQUIRE(std::any_cast<Out>(std::get<1>(ops[1])) == std::numeric_limits<In>::max());
}

TEST_CASE("Formatter works for built-in numeric types") {
    SECTION("short") {
        checkType<short, int>();
    }
    SECTION("unsigned short") {
        checkType<unsigned short, unsigned int>();
    }
    SECTION("int") {
        checkType<int, int>();
    }
    SECTION("unsigned int") {
        checkType<unsigned int, unsigned int>();
    }
    SECTION("long") {
      if constexpr (sizeof(long) == sizeof(int)) {
        checkType<long, int>();
      } else {
        checkType<long, long long>();
      }
    }
    SECTION("unsigned long") {
      if constexpr (sizeof(unsigned long) == sizeof(unsigned int)) {
        checkType<unsigned long, unsigned int>();
      } else {
        checkType<unsigned long, unsigned long long>();
      }
    }
    SECTION("long long") {
        checkType<long long, long long>();
    }
    SECTION("unsigned long long") {
        checkType<unsigned long long, unsigned long long>();
    }
    SECTION("float") {
        checkType<float, double>();
    }
    SECTION("double") {
        checkType<double, double>();
    }
    SECTION("long double") {
        checkType<long double, long double>();
    }
}

TEST_CASE("Formatter works for strings") {
  MockBuffer b;
  MockSerializer srl(&b);
  ops.clear();

  np::format(std::string_view("hello world"), srl);
  REQUIRE(ops.size() == 1);
  REQUIRE(std::get<0>(ops[0]) == "writeVal");
  REQUIRE(std::any_cast<std::string_view>(std::get<1>(ops[0])) == "hello world");
}

TEST_CASE("Formatter works for bools") {
  MockBuffer b;
  MockSerializer srl(&b);
  ops.clear();

  np::format(true, srl);
  np::format(false, srl);
  REQUIRE(ops.size() == 2);
  REQUIRE(std::get<0>(ops[0]) == "writeVal");
  REQUIRE(std::any_cast<bool>(std::get<1>(ops[0])));
  REQUIRE(std::get<0>(ops[0]) == "writeVal");
  REQUIRE(!std::any_cast<bool>(std::get<1>(ops[1])));
}

struct TestType {
  inline static int count = 0;
};
template <>
struct np::Formatter<TestType> {
  void operator()(const TestType& val, np::Serializer& srl) { ++TestType::count; }
};

TEST_CASE("Formatter can be extended for custom types") {
  MockBuffer b;
  MockSerializer srl(&b);
  ops.clear();

  np::format(TestType(), srl);
  REQUIRE(TestType::count == 1);
}
