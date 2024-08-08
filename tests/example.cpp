#include <iostream>
#include <nplog.hpp>
#include <nplog/config.hpp>

np::LogGroup mylog;

int main() {
  // Configure logger to include messages of level 3 and down, and properties of level 3 and down
  np::log::Config cfg;
  cfg.levels.default_level = {np::log::Info, np::log::Debug};
  cfg.sink = [](auto msg) { std::cout << msg.message << '\n'; };
  np::log::applyConfig(cfg);

  int fortytwo = 42;
  LOG(mylog, np::log::Info, "logging a message", WITH(fortytwo));
  LOG(mylog, np::log::Debug, "logging a message");
  LOG(mylog, np::log::Debug, "logging a message", WITH(np::log::Trace, fortytwo));
  LOG(mylog, np::log::Debug, "logging a message", WITH("myval", fortytwo));

  np::LogGroup otherlog(
    nullptr, nullptr, {{"name", std::string_view("yoyo")}, {"loggerprop1", 1.234}});
  LOG(otherlog, np::log::Info, "Wooosh", WITH("myval", fortytwo));
  LOG(otherlog, np::log::Info, "Wooosh2");
}
