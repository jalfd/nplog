#include <nplog.hpp>

np::Log mylog;

int main() {
  // Configure logger to include messages of level 3 and down, and parameters of level 3 and down
  np::Config cfg(5, 3);
  cfg.apply();

  np::setSink(np::getStdErrSink());

  int fortytwo = 42;
  LOG(mylog, 0, "logging a message", ARG(fortytwo));
  LOG(mylog, 0, "logging a message");
  LOG(mylog, 0, "logging a message", ARG(8, fortytwo));
  LOG(mylog, 0, "logging a message", ARG("myval", fortytwo));
}
