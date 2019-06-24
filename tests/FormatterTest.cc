#include <nplog/Formatter.hpp>
#include <numeric>
#include "mocks.hpp"
#include <catch/catch.hpp>

TEST_CASE("Formatter works for built-in numeric types") {
  SECTION("short") {
    std::vector<char> b;
    np::ValueSerializer srl(&b);
    np::format(std::numeric_limits<short>::min(), srl);
    REQUIRE(std::numeric_limits<short>::min() == std::stoi(std::string(b.begin(), b.end())));
  }
  SECTION("unsigned short") {
    std::vector<char> b;
    np::ValueSerializer srl(&b);
    np::format(std::numeric_limits<unsigned short>::min(), srl);
    REQUIRE(
      std::numeric_limits<unsigned short>::min() == std::stoul(std::string(b.begin(), b.end())));
  }
  SECTION("int") {
    std::vector<char> b;
    np::ValueSerializer srl(&b);
    np::format(std::numeric_limits<int>::min(), srl);
    REQUIRE(std::numeric_limits<int>::min() == std::stoi(std::string(b.begin(), b.end())));
  }
  SECTION("unsigned int") {
    std::vector<char> b;
    np::ValueSerializer srl(&b);
    np::format(std::numeric_limits<unsigned int>::min(), srl);
    REQUIRE(
      std::numeric_limits<unsigned int>::min() == std::stoul(std::string(b.begin(), b.end())));
  }
  SECTION("long") {
    std::vector<char> b;
    np::ValueSerializer srl(&b);
    np::format(std::numeric_limits<long>::min(), srl);
    REQUIRE(std::numeric_limits<long>::min() == std::stol(std::string(b.begin(), b.end())));
  }
  SECTION("unsigned long") {
    std::vector<char> b;
    np::ValueSerializer srl(&b);
    np::format(std::numeric_limits<unsigned long>::min(), srl);
    REQUIRE(
      std::numeric_limits<unsigned long>::min() == std::stoul(std::string(b.begin(), b.end())));
  }
  SECTION("long long") {
    std::vector<char> b;
    np::ValueSerializer srl(&b);
    np::format(std::numeric_limits<long long>::min(), srl);
    REQUIRE(std::numeric_limits<long long>::min() == std::stoll(std::string(b.begin(), b.end())));
  }
  SECTION("unsigned long long") {
    std::vector<char> b;
    np::ValueSerializer srl(&b);
    np::format(std::numeric_limits<unsigned long long>::min(), srl);
    REQUIRE(std::numeric_limits<unsigned long long>::min()
      == std::stoull(std::string(b.begin(), b.end())));
  }
  SECTION("float") {
    std::vector<char> b;
    np::ValueSerializer srl(&b);
    np::format(std::numeric_limits<float>::min(), srl);
    REQUIRE(
      Approx(std::numeric_limits<float>::min()) == std::stof(std::string(b.begin(), b.end())));
  }
  SECTION("double") {
    std::vector<char> b;
    np::ValueSerializer srl(&b);
    np::format(std::numeric_limits<double>::min(), srl);
    REQUIRE(
      Approx(std::numeric_limits<double>::min()) == std::stod(std::string(b.begin(), b.end())));
  }
  SECTION("long double") {
    std::vector<char> b;
    np::ValueSerializer srl(&b);
    np::format(std::numeric_limits<long double>::min(), srl);
    CAPTURE(std::string(b.begin(), b.end()));
    long double ld = std::strtold(std::string(b.begin(), b.end()).c_str(), nullptr);
    REQUIRE(Approx(std::numeric_limits<long double>::min()) == ld);
  }
}

TEST_CASE("Formatter works for strings") {
  std::vector<char> b;
  np::ValueSerializer srl(&b);
  ops.clear();

  np::format(std::string_view("hello world"), srl);
  REQUIRE(std::string(b.begin(), b.end()) == "\"hello world\"");
}

TEST_CASE("Formatter works for bools") {
  std::vector<char> b;
  np::ValueSerializer srl(&b);
  ops.clear();

  np::format(true, srl);
  REQUIRE(std::string(b.begin(), b.end()) == "true");
  b.clear();
  np::format(false, srl);
  REQUIRE(std::string(b.begin(), b.end()) == "false");
}

struct TestType {
  inline static int count = 0;
};
template <>
struct np::Formatter<TestType> {
  void operator()(const TestType&, np::ValueSerializer&) { ++TestType::count; }
};

TEST_CASE("Formatter can be extended for custom types") {
  std::vector<char> b;
  np::ValueSerializer srl(&b);
  ops.clear();

  np::format(TestType(), srl);
  REQUIRE(TestType::count == 1);
}
