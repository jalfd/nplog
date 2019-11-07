#include <nplog/config.hpp>
#include <nplog/serializer.hpp>
#include <picojson/picojson.h>
#include <regex>
#include <sstream>
#include <catch/catch.hpp>

namespace pj = picojson;

pj::object parseLogMessage(np::log::MessageBuffer buf) {
  pj::value val;
  std::string err;
  const auto contents = buf.contents();
  picojson::parse(val, contents.begin(), contents.end(), &err);
  CAPTURE(contents);
  REQUIRE(err == "");
  REQUIRE(val.is<pj::object>());
  return val.get<pj::object>();
}

template <typename T>
void checkFloatyValue(T val) {
  np::log::MessageBuffer buf;
  np::log::ValueSerializer vs(&buf);

  vs.write(val);

  std::regex fixed_float(R"(^-?\d+(\.\d+)?$)", std::regex_constants::ECMAScript);
  std::regex sci_float(R"(^-?\d+(\.\d+)?e[+-]?\d+$)", std::regex_constants::ECMAScript);
  const auto contents = buf.contents();
  CAPTURE(contents);
  bool is_fixed = std::regex_search(contents.begin(), contents.end(), fixed_float);
  bool is_sci = std::regex_search(contents.begin(), contents.end(), sci_float);
  REQUIRE((is_fixed || is_sci));

  std::istringstream istr((std::string(contents)));
  T result;
  istr >> result;
  REQUIRE(Approx(val) == result);
}

template <typename T>
void checkIntegralValue(T val) {
  np::log::MessageBuffer buf;
  np::log::ValueSerializer vs(&buf);

  vs.write(val);

  std::regex integral(R"(^-?\d+$)", std::regex_constants::ECMAScript);
  const auto contents = buf.contents();
  CAPTURE(contents);
  REQUIRE(std::regex_search(contents.begin(), contents.end(), integral));

  std::istringstream istr((std::string(contents)));
  T result;
  istr >> result;
  REQUIRE(val == result);
}

TEST_CASE("ValueSerializer") {
  SECTION("float") {
    checkFloatyValue<float>(0.0);
    checkFloatyValue<float>(-0.0);
    checkFloatyValue<float>(1.2345678f);
    checkFloatyValue<float>(-1.2345678f);
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
      np::log::MessageBuffer buf;
      np::log::ValueSerializer vs(&buf);
      vs.write(nan);
      REQUIRE(buf.messageSize() == 4);
      REQUIRE(buf.contents() == "null");
    }
    SECTION("Inf") {
      np::log::MessageBuffer buf;
      np::log::ValueSerializer vs(&buf);
      vs.write(inf);
      REQUIRE(buf.messageSize() == 4);
      REQUIRE(buf.contents() == "null");
    }
    SECTION("-Inf") {
      np::log::MessageBuffer buf;
      np::log::ValueSerializer vs(&buf);
      vs.write(-inf);
      REQUIRE(buf.messageSize() == 4);
      REQUIRE(buf.contents() == "null");
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
      np::log::MessageBuffer buf;
      np::log::ValueSerializer vs(&buf);
      vs.write(true);
      REQUIRE(buf.messageSize() == 4);
      REQUIRE(buf.contents() == "true");
    }
    SECTION("false") {
      np::log::MessageBuffer buf;
      np::log::ValueSerializer vs(&buf);
      vs.write(false);
      REQUIRE(buf.messageSize() == 5);
      REQUIRE(buf.contents() == "false");
    }
  }
  SECTION("string") {
    SECTION("empty string") {
      np::log::MessageBuffer buf;
      np::log::ValueSerializer vs(&buf);
      vs.write(std::string_view(""));
      REQUIRE(buf.contents() == R"("")");
    }
    SECTION("simple string") {
      np::log::MessageBuffer buf;
      np::log::ValueSerializer vs(&buf);
      vs.write(std::string_view("hello"));
      REQUIRE(buf.contents() == R"("hello")");
    }
    SECTION("string with quotes") {
      np::log::MessageBuffer buf;
      np::log::ValueSerializer vs(&buf);
      vs.write(std::string_view("hello \"world\""));
      REQUIRE(buf.contents() == "\"hello \\\"world\\\"\"");
    }
    SECTION("string with backslash") {
      np::log::MessageBuffer buf;
      np::log::ValueSerializer vs(&buf);
      vs.write(std::string_view("\\"));
      REQUIRE(buf.contents() == R"("\\")");
    }
    SECTION("string with non-ASCII characters") {
      np::log::MessageBuffer buf;
      np::log::ValueSerializer vs(&buf);
      vs.write(std::string_view("\u00d8"));
      REQUIRE(buf.contents() == "\"\u00d8\"");
    }
    SECTION("string with control characters") {
      np::log::MessageBuffer buf;
      np::log::ValueSerializer vs(&buf);
      vs.write(std::string_view("\a"));
      REQUIRE(buf.contents() == "\"\\u0007\"");
    }
    SECTION("string with whitespace") {
      np::log::MessageBuffer buf;
      np::log::ValueSerializer vs(&buf);
      vs.write(std::string_view("\t\r\n"));
      REQUIRE(buf.contents() == "\"\\t\\r\\n\"");
    }
    SECTION("string with null") {
      np::log::MessageBuffer buf;
      np::log::ValueSerializer vs(&buf);
      vs.write(std::string_view("\0", 1));
      REQUIRE(buf.contents() == "\"\\u0000\"");
    }
  }
}

TEST_CASE("Serializer") {
    np::log::Config cfg;
    cfg.fields = static_cast<np::log::Config::Fields>(-1);
    np::log::applyConfig(cfg);

  SECTION("Log with no parameters") {
    np::log::MessageBuffer buf;
    np::log::Serializer s(&buf);
    s.prologue("file.cc", 1, 2, {}, "msg");
    s.epilogue();

    std::string msg = buf.contents().data();
    CAPTURE(msg);
    auto result = parseLogMessage(std::move(buf));
    CHECK(result["file"].get<std::string>() == "file.cc");
    CHECK(result["line"].get<double>() == 1.0);
    CHECK(result["level"].get<double>() == 2.0);
    CHECK(result["message"].get<std::string>() == "msg");
    CHECK(result.find("params") == result.end());
  }
  SECTION("Log prologue is correctly encoded") {
    np::log::MessageBuffer buf;
    np::log::Serializer s(&buf);
    s.prologue("file\".cc", 1, 2, {}, "msg\\");
    s.epilogue();

    auto result = parseLogMessage(std::move(buf));
    CHECK(result["file"].get<std::string>() == "file\".cc");
    CHECK(result["message"].get<std::string>() == "msg\\");
  }
  SECTION("Log with one parameter") {
    np::log::MessageBuffer buf;
    np::log::Serializer s(&buf);
    s.prologue("file.cc", 1, 2, {}, "msg");
    s.writeKey("a");
    s.valueSerializer().write(3);
    s.epilogue();

    auto result = parseLogMessage(std::move(buf));
    CHECK(result["file"].get<std::string>() == "file.cc");
    CHECK(result["line"].get<double>() == 1.0);
    CHECK(result["level"].get<double>() == 2.0);
    CHECK(result["message"].get<std::string>() == "msg");
    auto params = result["params"].get<pj::object>();
    CHECK(params.size() == 1);
    CHECK(params["a"].get<double>() == 3.0);
  }
  SECTION("Log with multiple parameters") {
    np::log::MessageBuffer buf;
    np::log::Serializer s(&buf);
    s.prologue("file", 1, 2, {}, "msg");
    s.writeKey("a");
    s.valueSerializer().write(3);
    s.writeKey("b");
    s.valueSerializer().write(4);
    s.epilogue();

    auto result = parseLogMessage(std::move(buf));
    auto params = result["params"].get<pj::object>();
    CHECK(params.size() == 2);
    CHECK(params["a"].get<double>() == 3.0);
    CHECK(params["b"].get<double>() == 4.0);
  }
  SECTION("Parameter keys are correctly encoded") {
    np::log::MessageBuffer buf;
    np::log::Serializer s(&buf);
    s.prologue("file", 1, 2, {}, "msg");
    s.writeKey("\"");
    s.valueSerializer().write(3);
    s.epilogue();

    auto result = parseLogMessage(std::move(buf));
    auto params = result["params"].get<pj::object>();
    CHECK(params.size() == 1);
    CHECK(params["\""].get<double>() == 3.0);
  }
  SECTION("Serializer removes path from file") {
    SECTION("forward slashes") {
      np::log::MessageBuffer buf;
      np::log::Serializer s(&buf);
      s.prologue("foo/bar/file.cc", 1, 2, {}, "msg");
      s.epilogue();

      auto result = parseLogMessage(std::move(buf));
      CHECK(result["file"].get<std::string>() == "file.cc");
    }
    SECTION("backslashes") {
      np::log::MessageBuffer buf;
      np::log::Serializer s(&buf);
      s.prologue("foo\\bar\\file.cc", 1, 2, {}, "msg");
      s.epilogue();

      auto result = parseLogMessage(std::move(buf));
      CHECK(result["file"].get<std::string>() == "file.cc");
    }
  }
}
