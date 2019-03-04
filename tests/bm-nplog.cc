#include <nonius/nonius.h++>
#include <string>

#include <nplog/Log.hpp>
#include <nplog/ScopedMessage.hpp>
#include <nplog/macros.hpp>

namespace {
  np::Log nplog;
#ifdef _WIN32
  FILE* f = fopen("NUL", "w");
#else
  FILE* f = fopen("/dev/null", "w");
#endif

  int x = 42;
  double y = 127.003;
  std::string z = "this is a string parameter";

  int _ = []() {
    np::Log::setSink([](int, std::string_view msg) { fprintf(f, "%s\n", msg.data()); });
    np::Config cfg;
    cfg.default_levels.message = 3;
    cfg.default_levels.param = 3;
    cfg.apply();
    return 0;
  }();
}


NONIUS_BENCHMARK("nplog (simple message)", [] { LOG(nplog, 0, "this is a message"); })

NONIUS_BENCHMARK("nplog (suppressed message)", [] { LOG(nplog, 9, "this is a message"); })

NONIUS_BENCHMARK("nplog (with params)", [] { LOG(nplog, 0, "this is a message", ARG(x), ARG(y), ARG(z)); })

NONIUS_BENCHMARK("nplog (suppressed params)", [] { LOG(nplog, 0, "this is a message", ARG(9, x), ARG(9, y), ARG(9, z)); })

