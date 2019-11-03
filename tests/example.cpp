#include <nplog.hpp>
#include <iostream>

np::Logger mylog;

int main() {
  // Configure logger to include messages of level 3 and down, and parameters of level 3 and down
  np::log::Config cfg;
  cfg.levels.default_level = {5, 3};
  cfg.sink = [](auto msg) { std::cout << msg.message << '\n'; };
  np::log::applyConfig(cfg);

  int fortytwo = 42;
  LOG(mylog, 0, "logging a message", WITH(fortytwo));
  LOG(mylog, 0, "logging a message");
  LOG(mylog, 0, "logging a message", WITH(8, fortytwo));
  LOG(mylog, 0, "logging a message", WITH("myval", fortytwo));
}
