#include <nplog/macros.hpp>
#include <nplog/Log.hpp>
#include <nplog/ScopedMessage.hpp>
#include <nplog/Formatter.hpp>

np::Log mylog;

int main() {
    np::Log::setSink(np::getStdErrSink());
    int fortytwo = 42;
    LOG(mylog, 0, "logging a message", ARG(fortytwo));
    LOG(mylog, 0, "logging a message");
    LOG(mylog, 0, "logging a message", ARG(8, fortytwo));
    LOG(mylog, 0, "logging a message", ARG("myval", fortytwo));
}
