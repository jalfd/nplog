#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <string>

#include <nplog.hpp>
#include <nplog/config.hpp>
#include "util.h"

namespace
{
    void configureForField(np::log::Fields field_mask)
    {
        np::log::Config cfg;
        cfg.levels.default_level = {threshold(np::log::Status), threshold(np::log::Status)};
        cfg.sink = [](np::log::MessageInfo) {};
        cfg.fields = field_mask;
        np::log::applyConfig(cfg);
    }
} // namespace

TEST_CASE("Params", "[!benchmark]")
{
    BENCHMARK_ADVANCED("Params.Neither")
    (Catch::Benchmark::Chronometer cm)
    {
        configureForField(np::log::Fields());
        np::LogGroup nplog(nullptr, "My LogGroup");
        cm.measure([&]()
                   { repeat([&]()
                            { LOG(nplog, 0, ""); }); });
    };
    BENCHMARK_ADVANCED("Params.LogGroup")
    (Catch::Benchmark::Chronometer cm)
    {
        configureForField(np::log::Fields());
        np::LogGroup nplog(nullptr, "My LogGroup",
                           {{"first", 42}, {"second", 42}, {"third", 42}});
        cm.measure([&]()
                   { repeat([&]()
                            { LOG(nplog, 0, ""); }); });
    };
    BENCHMARK_ADVANCED("Params.Message")
    (Catch::Benchmark::Chronometer cm)
    {
        configureForField(np::log::Fields());
        np::LogGroup nplog(nullptr, "My LogGroup");
        cm.measure([&]()
                   { repeat([&]()
                            { LOG(nplog, 0, "", NP_WITH("first", 42), NP_WITH("second", 42), NP_WITH("third", 42)); }); });
    };
    BENCHMARK_ADVANCED("Params.Both")
    (Catch::Benchmark::Chronometer cm)
    {
        configureForField(np::log::Fields());
        np::LogGroup nplog(nullptr, "My LogGroup",
                           {{"fourth", 42}, {"fifth", 42}, {"sixth", 42}});
        cm.measure([&]()
                   { repeat([&]()
                            { LOG(nplog, 0, "", NP_WITH("first", 42), NP_WITH("second", 42), NP_WITH("third", 42)); }); });
    };
    BENCHMARK_ADVANCED("Params.Merge")
    (Catch::Benchmark::Chronometer cm)
    {
        configureForField(np::log::Fields());
        np::LogGroup nplog(nullptr, "My LogGroup",
                           {{"first", 42}, {"second", 42}, {"third", 42}});
        cm.measure([&]()
                   { repeat([&]()
                            { LOG(nplog, 0, "", NP_WITH("first", 42), NP_WITH("second", 42), NP_WITH("third", 42)); }); });
    };
}