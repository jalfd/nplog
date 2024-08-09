#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <string>

#include <nplog/serializer.hpp>
#include "../src/messagebuffer.hpp"
#include "util.h"

TEST_CASE("buffer", "[!benchmark]") {
  BENCHMARK_ADVANCED("Buffer.ControlChar")(Catch::Benchmark::Chronometer cm) {
    np::log::MessageBuffer buf;
    buf.insertAt(500);
    buf.clear();
    np::log::Serializer s(&buf);
    auto vs = s.valueSerializer();
    cm.measure([&] {
      repeat([&]() {
        vs.write("\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"");
        vs.write("\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"");
        vs.write("\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"");
        vs.write("\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"");
        vs.write("\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"");
        vs.write("\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"");
        vs.write("\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"");
        vs.write("\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"");
        vs.write("\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"");
        vs.write("\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"");
      });
    });
  };
  BENCHMARK_ADVANCED("Buffer.AsciiChar")(Catch::Benchmark::Chronometer cm) {
    np::log::MessageBuffer buf;
    buf.insertAt(500);
    buf.clear();
    np::log::Serializer s(&buf);
    auto vs = s.valueSerializer();
    cm.measure([&] {
      repeat([&]() {
        vs.write("xxxxxxxxxxxxxxxx");
        vs.write("xxxxxxxxxxxxxxxx");
        vs.write("xxxxxxxxxxxxxxxx");
        vs.write("xxxxxxxxxxxxxxxx");
        vs.write("xxxxxxxxxxxxxxxx");
        vs.write("xxxxxxxxxxxxxxxx");
        vs.write("xxxxxxxxxxxxxxxx");
        vs.write("xxxxxxxxxxxxxxxx");
        vs.write("xxxxxxxxxxxxxxxx");
        vs.write("xxxxxxxxxxxxxxxx");
      });
    });
  };
}