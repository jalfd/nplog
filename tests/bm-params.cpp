#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <string>

#include <nplog/config.hpp>
#include "util.h"
#include <nplog.hpp>

namespace {
  void configureForField(np::log::Fields field_mask) {
    np::log::Config cfg;
    cfg.levels.default_level = {np::log::Info, np::log::Info};
    cfg.sink = [](np::log::MessageInfo) {};
    cfg.fields = field_mask;
    np::log::applyConfig(cfg);
  }
} // namespace

TEST_CASE("Params", "[!benchmark]") {
  BENCHMARK_ADVANCED("Params.Neither")
  (Catch::Benchmark::Chronometer cm) {
    configureForField(np::log::Fields());
    np::LogGroup nplog(nullptr, "My LogGroup");
    cm.measure([&]() { repeat([&]() { LOG(nplog, 0, ""); }); });
  };
  BENCHMARK_ADVANCED("Params.LogGroup")
  (Catch::Benchmark::Chronometer cm) {
    configureForField(np::log::Fields());
    np::LogGroup nplog(nullptr, "My LogGroup");
    np::log::ScopedContext scoped0("first", 42);
    np::log::ScopedContext scoped1("second", 42);
    np::log::ScopedContext scoped2("third", 42);
    cm.measure([&]() { repeat([&]() { LOG(nplog, 0, ""); }); });
  };
  BENCHMARK_ADVANCED("Params.Message")
  (Catch::Benchmark::Chronometer cm) {
    configureForField(np::log::Fields());
    np::LogGroup nplog(nullptr, "My LogGroup");
    cm.measure([&]() {
      repeat([&]() {
        LOG(nplog, 0, "", NP_WITH("first", 42), NP_WITH("second", 42), NP_WITH("third", 42));
      });
    });
  };
  BENCHMARK_ADVANCED("Params.Both")
  (Catch::Benchmark::Chronometer cm) {
    configureForField(np::log::Fields());
    np::LogGroup nplog(nullptr, "My LogGroup");
    np::log::ScopedContext scoped0("fourth", 42);
    np::log::ScopedContext scoped1("fifth", 42);
    np::log::ScopedContext scoped2("sixth", 42);
    cm.measure([&]() {
      repeat([&]() {
        LOG(nplog, 0, "", NP_WITH("first", 42), NP_WITH("second", 42), NP_WITH("third", 42));
      });
    });
  };
}