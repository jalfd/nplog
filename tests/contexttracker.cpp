#include "../src/contexttracker.hpp"
#include <catch2/catch_test_macros.hpp>
#include <nplog/serializer.hpp>
#include <picojson/picojson.h>
#include <string>

namespace pj = picojson;

namespace Catch {
  template <>
  struct StringMaker<pj::value> {
    static std::string convert(pj::value const& value) { return value.serialize(); }
  };
} // namespace Catch

static pj::object parseContextFragment(std::string_view contents) {
  pj::value val;
  std::string err;
  const auto contents_object = '{' + std::string(contents) + '}';
  CAPTURE(contents);
  CAPTURE(contents_object);
  picojson::parse(val, contents_object.begin(), contents_object.end(), &err);
  REQUIRE(err == "");
  REQUIRE(val.is<pj::object>());
  return val.get<pj::object>();
}

TEST_CASE("ContextTracker") {
  GIVEN("A ContextTracker") {
    np::log::ContextTracker ct;

    WHEN("a context entry is inserted") {
      np::log::ValueSerializer vs;
      const auto id = ct.allocate("foo", &vs);
      vs.write(42);
      THEN("the context is an object containing just that entry") {
        const auto result = parseContextFragment(ct.context().contents());
        CHECK(result.size() == 1);
        CHECK(result.at("foo") == pj::value(42.0));
      }

      AND_WHEN("the context entry is removed") {
        ct.release(id);
        THEN("the context is empty") {
          const auto result = parseContextFragment(ct.context().contents());
          CHECK(result.size() == 0);
        }
      }

      AND_WHEN("another context entry is inserted") {
        const auto id2 = ct.allocate("bar", &vs);
        vs.write("hello");

        THEN("the context contains both") {
          const auto result = parseContextFragment(ct.context().contents());
          CHECK(result.size() == 2);
          CHECK(result.at("foo") == pj::value(42.0));
          CHECK(result.at("bar") == pj::value("hello"));
        }

        AND_WHEN("the first context entry is removed") {
          ct.release(id);
          THEN("the context contains only the second entry") {
            const auto result = parseContextFragment(ct.context().contents());
            CHECK(result.size() == 1);
            CHECK(result.at("bar") == pj::value("hello"));
          }
        }
      }

      AND_WHEN("the same context entry is inserted again") {
        const auto id2 = ct.allocate("foo", &vs);
        vs.write("hello");
        THEN("a different ID is returned") { CHECK(id != id2); }
        AND_THEN("the context contains only the most recent entry") {
          const auto result = parseContextFragment(ct.context().contents());
          CHECK(result.size() == 1);
          CHECK(result.at("foo") == pj::value("hello"));
        }

        AND_WHEN("the first entry is removed") {
          ct.release(id);
          THEN("the context contains only the second entry") {
            const auto result = parseContextFragment(ct.context().contents());
            CHECK(result.size() == 1);
            CHECK(result.at("foo") == pj::value("hello"));
          }
        }
        AND_WHEN("the second entry is removed") {
          ct.release(id2);
          THEN("the context contains only the first entry") {
            const auto result = parseContextFragment(ct.context().contents());
            CHECK(result.size() == 1);
            CHECK(result.at("foo") == pj::value(42.0));
          }
        }
      }
    }

    WHEN("a nonexistent entry is removed") {
      ct.release(923);
      THEN("the context is empty") {
        const auto result = parseContextFragment(ct.context().contents());
        CHECK(result.size() == 0);
      }
    }
  }
}
