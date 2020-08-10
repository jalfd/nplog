#include <iostream>
#include <nplog.hpp>

np::Logger mylog;

int main() {
  // Configure logger to include messages of level 3 and down, and parameters of level 3 and down
  np::log::Config cfg;
  cfg.levels.default_level = {threshold(np::log::DebugLow), threshold(np::log::Status)};
  cfg.sink = [](auto msg) { std::cout << msg.message << '\n'; };
  np::log::applyConfig(cfg);

  int fortytwo = 42;
  LOG(mylog, np::log::Status, "logging a message", WITH(fortytwo));
  LOG(mylog, np::log::DebugLow, "logging a message");
  LOG(mylog, np::log::DebugLow, "logging a message", WITH(np::log::Trace, fortytwo));
  LOG(mylog, np::log::DebugLow, "logging a message", WITH("myval", fortytwo));

  np::Logger otherlog(
    nullptr, nullptr, {{"name", std::string_view("yoyo")}, {"loggerparam1", 1.234}});
  LOG(otherlog, np::log::Status, "Wooosh", WITH("myval", fortytwo));
  LOG(otherlog, np::log::Status, "Wooosh2");
}
