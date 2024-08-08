#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <string>

#include <nplog/loggroup.hpp>
#include <nplog/config.hpp>
#include <nplog/scopedmessage.hpp>
#include <nplog/macros.hpp>
#include "util.h"

namespace
{
  constexpr auto default_fields = static_cast<np::log::Fields>(
      np::log::File | np::log::Line | np::log::Time |
      np::log::LevelName | np::log::ProcessId | np ::log::ThreadId);

  np::LogGroup nplog;
#ifdef _WIN32
  FILE *npfopen(const char *file, const char *mode)
  {
    FILE *f = nullptr;
    fopen_s(&f, file, mode);
    return f;
  }

  FILE *f = npfopen(NULLFILE, "w");

#else
  FILE *f = fopen(NULLFILE, "w");
#endif

  int x = 42;
  double y = 127.003;
  std::string z = "this is a string parameter";

  np::log::Config root_cfg = []()
  {
    np::log::Config cfg;
    cfg.levels.default_level = {threshold(np::log::Status), threshold(np::log::Status)};
    cfg.sink = [](np::log::MessageInfo mi)
    { fwrite(mi.message.data(), sizeof(char), mi.message.size(), f); };
    cfg.fields = static_cast<np::log::Fields>(-1);
    return cfg;
  }();
} // namespace

TEST_CASE("nplog", "[!benchmark]")
{
  BENCHMARK_ADVANCED("Log.Nplog.Simple")
  (Catch::Benchmark::Chronometer cm)
  {
    np::log::Config cfg = root_cfg;
    cfg.fields = default_fields;
    np::log::applyConfig(cfg);
    cm.measure([]
               { repeat([&]()
                        { LOG(nplog, 0, "this is a message"); }); });
  };

  BENCHMARK_ADVANCED("Log.Nplog.Params")
  (Catch::Benchmark::Chronometer cm)
  {
    np::log::Config cfg = root_cfg;
    cfg.fields = default_fields;
    np::log::applyConfig(cfg);
    cm.measure(
        []
        { repeat([&]()
                 { LOG(nplog, 0, "this is a message", WITH(x), WITH(y), WITH(z)); }); });
  };

  BENCHMARK_ADVANCED("Log.Nplog.SuppressedMessage")
  (Catch::Benchmark::Chronometer cm)
  {
    np::log::Config cfg = root_cfg;
    cfg.fields = default_fields;
    np::log::applyConfig(cfg);
    cm.measure([]
               { repeat([&]()
                        { LOG(nplog, 9, "this is a message"); }); });
  };

  BENCHMARK_ADVANCED("Log.Nplog.SuppressedParams")
  (Catch::Benchmark::Chronometer cm)
  {
    np::log::Config cfg = root_cfg;
    cfg.fields = default_fields;
    np::log::applyConfig(cfg);
    cm.measure([]
               { repeat([&]()
                        { LOG(nplog, 0, "this is a message", WITH(9, x), WITH(9, y), WITH(9, z)); }); });
  };
}