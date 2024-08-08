#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <string>

#include <nplog.hpp>
#include <nplog/config.hpp>
#include "util.h"

namespace
{
    np::LogGroup nplog("My Logger");

    void configureForField(np::log::Fields field_mask)
    {
        np::log::Config cfg;
        cfg.levels.default_level = {threshold(np::log::Status), threshold(np::log::Status)};
        cfg.sink = [](np::log::MessageInfo) {
        };
        cfg.fields = field_mask;
        np::log::applyConfig(cfg);
    }

    void logMessage()
    {
        LOG(nplog, 0, "");
    }
} // namespace

TEST_CASE("headers", "[!benchmark]")
{
    BENCHMARK_ADVANCED("Header.None")
    (Catch::Benchmark::Chronometer cm)
    {
        configureForField(np::log::Fields());
        cm.measure([]()
                   { repeat(logMessage); });
    };
    BENCHMARK_ADVANCED("Header.File")
    (Catch::Benchmark::Chronometer cm)
    {
        configureForField(np::log::File);
        cm.measure([]()
                   { repeat(logMessage); });
    };
    BENCHMARK_ADVANCED("Header.Line")
    (Catch::Benchmark::Chronometer cm)
    {
        configureForField(np::log::Line);
        cm.measure([]()
                   { repeat(logMessage); });
    };
    BENCHMARK_ADVANCED("Header.Time")
    (Catch::Benchmark::Chronometer cm)
    {
        configureForField(np::log::Time);
        cm.measure([]()
                   { repeat(logMessage); });
    };
    BENCHMARK_ADVANCED("Header.Level")
    (Catch::Benchmark::Chronometer cm)
    {
        configureForField(np::log::Level);
        cm.measure([]()
                   { repeat(logMessage); });
    };
    BENCHMARK_ADVANCED("Header.LevelName")
    (Catch::Benchmark::Chronometer cm)
    {
        configureForField(np::log::LevelName);
        cm.measure([]()
                   { repeat(logMessage); });
    };
    BENCHMARK_ADVANCED("Header.LogName")
    (Catch::Benchmark::Chronometer cm)
    {
        configureForField(np::log::LogName);
        cm.measure([]()
                   { repeat(logMessage); });
    };
    BENCHMARK_ADVANCED("Header.ProcessName")
    (Catch::Benchmark::Chronometer cm)
    {
        configureForField(np::log::ProcessName);
        cm.measure([]()
                   { repeat(logMessage); });
    };
    BENCHMARK_ADVANCED("Header.ProcessId")
    (Catch::Benchmark::Chronometer cm)
    {
        configureForField(np::log::ProcessId);
        cm.measure([]()
                   { repeat(logMessage); });
    };
    BENCHMARK_ADVANCED("Header.ThreadId")
    (Catch::Benchmark::Chronometer cm)
    {
        configureForField(np::log::ThreadId);
        cm.measure([]()
                   { repeat(logMessage); });
    };
    BENCHMARK_ADVANCED("Header.Hostname")
    (Catch::Benchmark::Chronometer cm)
    {
        configureForField(np::log::Hostname);
        cm.measure([]()
                   { repeat(logMessage); });
    };
}