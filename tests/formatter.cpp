#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <nplog/formatter.hpp>
#include <nplog/serializer.hpp>
#include <numeric>
#include "../src/messagebuffer.hpp"

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
    REQUIRE(std::numeric_limits<unsigned short>::min() == std::stoul(std::string(b.contents())));
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
    REQUIRE(std::numeric_limits<unsigned int>::min() == std::stoul(std::string(b.contents())));
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
    REQUIRE(std::numeric_limits<unsigned long>::min() == std::stoul(std::string(b.contents())));
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
    REQUIRE(
      std::numeric_limits<unsigned long long>::min() == std::stoull(std::string(b.contents())));
  }
  SECTION("float") {
    np::log::MessageBuffer b;
    np::log::ValueSerializer srl(&b);
    np::log::format(std::numeric_limits<float>::min(), srl);
    REQUIRE(
      Catch::Approx(std::numeric_limits<float>::min()) == std::stof(std::string(b.contents())));
  }
  SECTION("double") {
    np::log::MessageBuffer b;
    np::log::ValueSerializer srl(&b);
    np::log::format(std::numeric_limits<double>::min(), srl);
    REQUIRE(
      Catch::Approx(std::numeric_limits<double>::min()) == std::stod(std::string(b.contents())));
  }
  SECTION("long double") {
    np::log::MessageBuffer b;
    np::log::ValueSerializer srl(&b);
    np::log::format(std::numeric_limits<long double>::min(), srl);
    CAPTURE(b.contents());
    long double ld = std::strtold(std::string(b.contents()).c_str(), nullptr);
    REQUIRE(Catch::Approx(std::numeric_limits<long double>::min()) == ld);
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

TEST_CASE("Missing Functionality") {
  SKIP("Not implemented yet");
  SECTION("ValueSerializer accepts char literals") {
    np::log::MessageBuffer b;
    np::log::ValueSerializer srl(&b);
    // srl.writeLiteral('x');
    REQUIRE(std::string(b.contents()) == "x");
  }

  SECTION("ValueSerializer writes arrays") {
    SKIP("Not implemented yet");
    np::log::MessageBuffer b;
    np::log::ValueSerializer srl(&b);
    // srl.startArray();
    // srl.writeArrayElement(41);
    // srl.writeArrayElement("42");
    // srl.endArray();
    REQUIRE(std::string(b.contents()) == "[41,\"42\"]");
  }

  SECTION("ValueSerializer writes nested custom types") {
    SKIP("Not implemented yet");
    np::log::MessageBuffer b;
    np::log::ValueSerializer srl(&b);
    // srl.startArray();
    // srl.writeArrayElement(TestType());
    // srl.endArray();
    REQUIRE(std::string(b.contents()) == "[]");
  }
}
