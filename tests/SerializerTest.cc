#include <nplog/Serializer.hpp>
#include <regex>
#include <sstream>
#include <catch/catch.hpp>

template <typename T>
void checkFloatyValue(T val) {
  std::vector<char> buf;
  np::ValueSerializer vs(&buf);

  vs.write(val);

  std::regex fixed_float(R"(^-?\d+(\.\d+)?$)", std::regex_constants::ECMAScript);
  std::regex sci_float(R"(^-?\d+(\.\d+)?e[+-]?\d+$)", std::regex_constants::ECMAScript);
  CAPTURE(std::string(buf.begin(), buf.end()));
  bool is_fixed = std::regex_search(buf.begin(), buf.end(), fixed_float);
  bool is_sci = std::regex_search(buf.begin(), buf.end(), sci_float);
  REQUIRE((is_fixed || is_sci));

  std::istringstream istr(std::string(buf.begin(), buf.end()));
  T result;
  istr >> result;
  REQUIRE(Approx(val) == result);
}

template <typename T>
void checkIntegralValue(T val) {
  std::vector<char> buf;
  np::ValueSerializer vs(&buf);

  vs.write(val);

  std::regex integral(R"(^-?\d+$)", std::regex_constants::ECMAScript);
  CAPTURE(std::string(buf.begin(), buf.end()));
  REQUIRE(std::regex_search(buf.begin(), buf.end(), integral));

  std::istringstream istr(std::string(buf.begin(), buf.end()));
  T result;
  istr >> result;
  REQUIRE(val == result);
}

TEST_CASE("ValueSerializer") {
  SECTION("float") {
    checkFloatyValue<float>(0.0);
    checkFloatyValue<float>(-0.0);
    checkFloatyValue<float>(1.2345678);
    checkFloatyValue<float>(-1.2345678);
    checkFloatyValue<float>(std::numeric_limits<float>::max());
  }
  SECTION("double") {
    checkFloatyValue<double>(0.0);
    checkFloatyValue<double>(-0.0);
    checkFloatyValue<double>(1.2345678);
    checkFloatyValue<double>(-1.2345678);
    checkFloatyValue<double>(std::numeric_limits<double>::max());
  }
  SECTION("long double") {
    checkFloatyValue<long double>(0.0);
    checkFloatyValue<long double>(-0.0);
    checkFloatyValue<long double>(1.2345678);
    checkFloatyValue<long double>(-1.2345678);
    checkFloatyValue<long double>(std::numeric_limits<long double>::max());
  }
  SECTION("special IEEE values") {
    auto nan = std::numeric_limits<float>::quiet_NaN();
    auto inf = std::numeric_limits<float>::infinity();

    SECTION("NaN") {
      std::vector<char> buf;
      np::ValueSerializer vs(&buf);
      vs.write(nan);
      REQUIRE(buf.size() == 4);
      REQUIRE(std::string_view(&buf[0], 4) == "null");
    }
    SECTION("Inf") {
      std::vector<char> buf;
      np::ValueSerializer vs(&buf);
      vs.write(inf);
      REQUIRE(buf.size() == 4);
      REQUIRE(std::string_view(&buf[0], 4) == "null");
    }
    SECTION("-Inf") {
      std::vector<char> buf;
      np::ValueSerializer vs(&buf);
      vs.write(-inf);
      REQUIRE(buf.size() == 4);
      REQUIRE(std::string_view(&buf[0], 4) == "null");
    }
  }
  SECTION("int") {
    checkIntegralValue<int>(0);
    checkIntegralValue<int>(std::numeric_limits<int>::max());
    checkIntegralValue<int>(std::numeric_limits<int>::min());
  }
  SECTION("long") {
    checkIntegralValue<long>(0);
    checkIntegralValue<long>(std::numeric_limits<long>::max());
    checkIntegralValue<long>(std::numeric_limits<long>::min());
  }
  SECTION("long long") {
    checkIntegralValue<long long>(0);
    checkIntegralValue<long long>(std::numeric_limits<long long>::max());
    checkIntegralValue<long long>(std::numeric_limits<long long>::min());
  }
  SECTION("unsigned int") {
    checkIntegralValue<unsigned int>(0);
    checkIntegralValue<unsigned int>(std::numeric_limits<unsigned int>::max());
  }
  SECTION("unsigned long") {
    checkIntegralValue<unsigned long>(0);
    checkIntegralValue<unsigned long>(std::numeric_limits<unsigned long>::max());
  }
  SECTION("unsigned long long") {
    checkIntegralValue<unsigned long long>(0);
    checkIntegralValue<unsigned long long>(std::numeric_limits<unsigned long long>::max());
  }
  SECTION("bool") {
    SECTION("true") {
      std::vector<char> buf;
      np::ValueSerializer vs(&buf);
      vs.write(true);
      REQUIRE(buf.size() == 4);
      REQUIRE(std::string_view(&buf[0], 4) == "true");
    }
    SECTION("false") {
      std::vector<char> buf;
      np::ValueSerializer vs(&buf);
      vs.write(false);
      REQUIRE(buf.size() == 5);
      REQUIRE(std::string_view(&buf[0], 5) == "false");
    }
  }
  SECTION("string") {
    SECTION("empty string") {
      std::vector<char> buf;
      np::ValueSerializer vs(&buf);
      vs.write(std::string_view(""));
      std::string result(buf.begin(), buf.end());
      REQUIRE(result == R"("")");
    }
    SECTION("simple string") {
      std::vector<char> buf;
      np::ValueSerializer vs(&buf);
      vs.write(std::string_view("hello"));
      std::string result(buf.begin(), buf.end());
      REQUIRE(result == R"("hello")");
    }
    SECTION("string with quotes") {
      std::vector<char> buf;
      np::ValueSerializer vs(&buf);
      vs.write(std::string_view("hello \"world\""));
      std::string result(buf.begin(), buf.end());
      REQUIRE(result == R"("hello \"world\"")");
    }
    SECTION("string with backslash") {
      std::vector<char> buf;
      np::ValueSerializer vs(&buf);
      vs.write(std::string_view("\\"));
      std::string result(buf.begin(), buf.end());
      REQUIRE(result == R"("\\")");
    }
    SECTION("string with non-ASCII characters") {
      std::vector<char> buf;
      np::ValueSerializer vs(&buf);
      vs.write(std::string_view("\u00d8"));
      std::string result(buf.begin(), buf.end());
      REQUIRE(result == "\"\u00d8\"");
    }
    SECTION("string with control characters") {
      std::vector<char> buf;
      np::ValueSerializer vs(&buf);
      vs.write(std::string_view("\a"));
      std::string result(buf.begin(), buf.end());
      REQUIRE(result == "\"\\u0007\"");
    }
    SECTION("string with whitespace") {
      std::vector<char> buf;
      np::ValueSerializer vs(&buf);
      vs.write(std::string_view("\t\r\n"));
      std::string result(buf.begin(), buf.end());
      REQUIRE(result == "\"\\t\\r\\n\"");
    }
    SECTION("string with null") {
      std::vector<char> buf;
      np::ValueSerializer vs(&buf);
      vs.write(std::string_view("\0", 1));
      std::string result(buf.begin(), buf.end());
      REQUIRE(result == "\"\\u0000\"");
    }
  }
}

TEST_CASE("Serializer") {
    SECTION("Log with no parameters") {
        std::vector<char> buf;
        np::Serializer s(&buf);
        s.prologue("file", 1, 2, "msg");
        s.epilogue();

      std::string result(buf.begin(), buf.end());
      REQUIRE(result == R"({"file":"file","line":1,"level":2,"message":"msg"})");
    }
    SECTION("Log header is correctly encoded") {
        std::vector<char> buf;
        np::Serializer s(&buf);
        s.prologue("\"", 1, 2, "\"");
        s.epilogue();

      std::string result(buf.begin(), buf.end());
      REQUIRE(result == R"({"file":"\"","line":1,"level":2,"message":"\""})");
    }
    SECTION("Log with one parameter") {
        std::vector<char> buf;
        np::Serializer s(&buf);
        s.prologue("file", 1, 2, "msg");
        s.writeKey("a");
        s.valueSerializer().write(3);
        s.epilogue();

      std::string result(buf.begin(), buf.end());
      REQUIRE(result == R"({"file":"file","line":1,"level":2,"message":"msg","params":{"a":3}})");
    }
    SECTION("Log with multiple parameters") {
        std::vector<char> buf;
        np::Serializer s(&buf);
        s.prologue("file", 1, 2, "msg");
        s.writeKey("a");
        s.valueSerializer().write(3);
        s.writeKey("b");
        s.valueSerializer().write(4);
        s.epilogue();

      std::string result(buf.begin(), buf.end());
      REQUIRE(result == R"({"file":"file","line":1,"level":2,"message":"msg","params":{"a":3,"b":4}})");
    }
    SECTION("Parameter keys are correctly encoded") {
        std::vector<char> buf;
        np::Serializer s(&buf);
        s.prologue("file", 1, 2, "msg");
        s.writeKey("\"");
        s.valueSerializer().write(3);
        s.epilogue();

      std::string result(buf.begin(), buf.end());
      REQUIRE(result == R"({"file":"file","line":1,"level":2,"message":"msg","params":{"\"":3}})");
    }
    SECTION("Serializer removes path from file") {
      SECTION("forward slashes") {
        std::vector<char> buf;
        np::Serializer s(&buf);
        s.prologue("foo/bar/file.cc", 1, 2, "msg");
        s.epilogue();

        std::string result(buf.begin(), buf.end());
        REQUIRE(result == R"({"file":"file.cc","line":1,"level":2,"message":"msg"})");
      }
      SECTION("backslashes") {
        std::vector<char> buf;
        np::Serializer s(&buf);
        s.prologue("foo\\bar\\file.cc", 1, 2, "msg");
        s.epilogue();

        std::string result(buf.begin(), buf.end());
        REQUIRE(result == R"({"file":"file.cc","line":1,"level":2,"message":"msg"})");
      }
    }
}
