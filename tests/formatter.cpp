#include <nplog/formatter.hpp>
#include <nplog/messagebuffer.hpp>
#include <nplog/serializer.hpp>
#include <numeric>
#include <catch/catch.hpp>

TEST_CASE("Formatter works for built-in numeric types") {
  SECTION("short") {
    np::log::MessageBuffer b;
    np::log::ValueSerializer srl(&b);
    np::log::format(std::numeric_limits<short>::min(), srl);
    REQUIRE(std::numeric_limits<short>::min() == std::stoi(std::string(b.contents())));
  }
  SECTION("unsigned short") {
    np::log::MessageBuffer b;
    np::log::ValueSerializer srl(&b);
    np::log::format(std::numeric_limits<unsigned short>::min(), srl);
    REQUIRE(
      std::numeric_limits<unsigned short>::min() == std::stoul(std::string(b.contents())));
  }
  SECTION("int") {
    np::log::MessageBuffer b;
    np::log::ValueSerializer srl(&b);
    np::log::format(std::numeric_limits<int>::min(), srl);
    REQUIRE(std::numeric_limits<int>::min() == std::stoi(std::string(b.contents())));
  }
  SECTION("unsigned int") {
    np::log::MessageBuffer b;
    np::log::ValueSerializer srl(&b);
    np::log::format(std::numeric_limits<unsigned int>::min(), srl);
    REQUIRE(
      std::numeric_limits<unsigned int>::min() == std::stoul(std::string(b.contents())));
  }
  SECTION("long") {
    np::log::MessageBuffer b;
    np::log::ValueSerializer srl(&b);
    np::log::format(std::numeric_limits<long>::min(), srl);
    REQUIRE(std::numeric_limits<long>::min() == std::stol(std::string(b.contents())));
  }
  SECTION("unsigned long") {
    np::log::MessageBuffer b;
    np::log::ValueSerializer srl(&b);
    np::log::format(std::numeric_limits<unsigned long>::min(), srl);
    REQUIRE(
      std::numeric_limits<unsigned long>::min() == std::stoul(std::string(b.contents())));
  }
  SECTION("long long") {
    np::log::MessageBuffer b;
    np::log::ValueSerializer srl(&b);
    np::log::format(std::numeric_limits<long long>::min(), srl);
    REQUIRE(std::numeric_limits<long long>::min() == std::stoll(std::string(b.contents())));
  }
  SECTION("unsigned long long") {
    np::log::MessageBuffer b;
    np::log::ValueSerializer srl(&b);
    np::log::format(std::numeric_limits<unsigned long long>::min(), srl);
    REQUIRE(std::numeric_limits<unsigned long long>::min()
      == std::stoull(std::string(b.contents())));
  }
  SECTION("float") {
    np::log::MessageBuffer b;
    np::log::ValueSerializer srl(&b);
    np::log::format(std::numeric_limits<float>::min(), srl);
    REQUIRE(
      Approx(std::numeric_limits<float>::min()) == std::stof(std::string(b.contents())));
  }
  SECTION("double") {
    np::log::MessageBuffer b;
    np::log::ValueSerializer srl(&b);
    np::log::format(std::numeric_limits<double>::min(), srl);
    REQUIRE(
      Approx(std::numeric_limits<double>::min()) == std::stod(std::string(b.contents())));
  }
  SECTION("long double") {
    np::log::MessageBuffer b;
    np::log::ValueSerializer srl(&b);
    np::log::format(std::numeric_limits<long double>::min(), srl);
    CAPTURE(b.contents());
    long double ld = std::strtold(std::string(b.contents()).c_str(), nullptr);
    REQUIRE(Approx(std::numeric_limits<long double>::min()) == ld);
  }
}

TEST_CASE("Formatter works for strings") {
  np::log::MessageBuffer b;
  np::log::ValueSerializer srl(&b);

  np::log::format(std::string_view("hello world"), srl);
  REQUIRE(b.contents() == "\"hello world\"");
}

TEST_CASE("Formatter works for bools") {
  np::log::MessageBuffer b;
  np::log::ValueSerializer srl(&b);

  np::log::format(true, srl);
  REQUIRE(b.contents() == "true");
  b.clear();
  np::log::format(false, srl);
  REQUIRE(b.contents() == "false");
}

struct TestType {
  inline static int count = 0;
};
template <>
struct np::log::Formatter<TestType> {
  void operator()(const TestType&, np::log::ValueSerializer&) { ++TestType::count; }
};

TEST_CASE("Formatter can be extended for custom types") {
  np::log::MessageBuffer b;
  np::log::ValueSerializer srl(&b);

  np::log::format(TestType(), srl);
  REQUIRE(TestType::count == 1);
}
