# Getting Started

## Basic Usage
Every project uses logging differently. How do you want to configure logging in your application? Some logging frameworks just read a certain file assuming a certain format. Others check a specific environment variable. Nplog instead makes configuration part of the API. How you get the configuration is up to you. You can hardcode it, or you can read it from your own config files using your own format, or download it from a central server or whatever you like. But you have to write the code to call the config-related API functions.

This might seem a bit tedious and verbose, but it is just a handful of lines of code, and you only have to write just once, to fit your specific needs. And it means the logging library doesn't contain any hidden code that interacts with the environment in unexpected ways. It won't suddenly behave differently just because a certain magic environment variable was set, and it won't try to read stuff from your local file system. It expects you to tell it what it needs.

With that said, let's get started. In order to use nplog, you'll need to do the following


### Configuration
At some point, probably during application startup, you need to configure the nplog library. This is done by calling the function `np::log::applyConfig()`. This function takes an argument of type `np::log::Config`, which holds all the configuration options you want to set. For example, you might do something like this at the top of your `main()`:

```c++
// main.cc
#include <nplog/config.hpp> // needed to be able to configure nplog

int main() {
    np::log::Config cfg;
    // Log messages should be skipped if their severity is below DebugHigh
    cfg.levels.default_level = {threshold(np::log::DebugHigh)};
    // When a message is logged, write it to stdout
    cfg.sink = [](auto msg) { std::cout << msg.message << '\n'; };

    // apply the specified configuration
    np::log::applyConfig(cfg);
}
```

Here, we create a Config object, and on it, we set two properties: We specify the minimum log message severity (this is named the *default* level for reasons that will become clear later), effectively saying all messages with a lower severity should be discarded.

We also set the `sink` field. This is nothing more than a `std::function` value which is called with each message that should be logged. Again, many logging libraries allow you to use predefined sinks to say "I want to log to stderr", or "I want to log to a file with this name". And if you then want to do something different and unanticipated, you have to write your own code.

nplog just delegates this responsibility to you. Here's the message, do with it what you want!


### Common LogGroup

In order to log messages, we also need a `LogGroup`. The purpose of this will be explained later, but you do need one. Luckily, they're easy to create, and for simple use cases, we can just create one single global `LogGroup`, and use that for everything.

So let's create a single header that we'll use any time we want to log anything:

```c++
// my_nplog_wrapper.h
#ifndef MY_NPLOG_WRAPPER_H
#define MY_NPLOG_WRAPPER_H
#include <nplog.hpp>

inline np::log::LogGroup log_group;

#endif
```

To keep this example minimal, we just declared the variable `inline`. Perhaps you instead want to declare it `extern` and instantiate it in a single source file. Or perhaps you want a function that returns a reference to a static instance. You can do that too.

### Logging
Finally, we're ready to log some stuff.
We include the common header we just created, and then we invoke the `LOG` macro:

```c++
// somefunction.cc
#include "my_nplog_wrapper.h"

void someFunction() {
    LOG(log_group, np::log::Status, "someFunction called");
}
```

What happens now when someFunction is called?

We get the following written to stdout:

```
{"message":"someFunction called","file":"somefunction.cc","line":5,"time":"2020-11-23T11:56:47.793Z","levelString":"Status"}
```

There are a few things worth noting here:

* As you'd expect from any decent logging library, it tells us which file and line the messge was logged from.
* The time at which the message was logged is included as well, as an ISO 8601 time string showing the UTC time.
* The whole message is formatted as JSON

That's a start, but plenty of libraries can do this.

## Adding properties to log messages
Being able to log a single string is not very useful. Usually, we want to log dynamic data. We want to include the values of variables.

Traditionally, this has been done by embedding these values in the string message. For example,

```c++
int x = 42;
std::clog << "the value of x is " << x << '\n';
```

This seems simple, but it has some downsides:

* St makes it difficult to do any kind of bulk processing of log messages. In the above example, suppose we want to get all the different values of `x` that have been logged. We'll need to resort to regular expressions or other string processing means.
* it burdens the programmer with trying to write cumbersome messages that include all the values they want to log. For example, we may have to do something like:
    ```c++
    std::clog << "Sending message " << msg << " to server " << hostname << ". Server responded with " << response_code << " and message " << response << '\n';
    ```
    which will log a message like this: `Sending message FOO to server google.com. Server responded with 200 and response OK`

Nplog instead uses *structured logging*. All log messages are output as JSON objects, where one property contains the message itself is a single fixed string, but you can then associate any number of additional named properties with the message. For example, the above can be rewritten using nplog:

```c++
LOG(log_group, np::log::Status, "Sending message to server", WITH(msg), WITH(hostname), WITH(response_code));
```
which will result in the following log message:

`{"message":"Sending message to server","props":{"msg": "FOO","hostname":"google.com","response_code":200,"response":"OK"}}`

This has several advantages:

* It makes it clear to the reader of the log file which parts of the message are static, and which describe variable data.
* It frees the programmer from having to weave all the parameters into a single message. We can write a short description of what operation we're performing, and just add properties describing the necessary pieces of program state. The message itself doesn't have to include a note that the server sent back a response code and a response message, because the properties named `response_code` and `response` express this much more succinctly.
* Because these parameters are represented as separate JSON properties, we can easily query for them in log files. We can easily find all the different hostnames that we've sent messages to, or find out how many responded with anything other than "200 OK"

Of course, sometimes you might want to override the name given to a property. In that case, simply call WITH with two parameters instead: `WITH(fortytwo, 42)`. This will create a property with the name `"fortytwo"` and the value `42`.

At this point, we've pretty much covered the functionality you typically associate with a logging library. You can log messages along with some variable data. You can specify a severity for a message, and a severity threshold at which the message is discarded instead of being logged. And you can specify where messages should be sent once written.

But nplog has a few other cool tricks up its sleeve.

## Suppressing properties
Sometimes, you want to log data that might be really expensive. Perhaps you want to add a stack trace to the log message, so you can see the entire call stack. But that's not a cheap operation, so if we do this, we need to have a way to *turn it off* as well.

nplog supports this by assigning a severity level not just to the message, but to each individual property. As an example, let's reuse the example from before, but add a stack trace property:

```c++
LOG(log_group, np::log::Status, "Sending message to server", WITH(msg), WITH(hostname), WITH(response_code), WITH(stacktrace, getStackTrace()));
```

As it is, the entire message, with all of its properties, will be logged if the log severity threshold is set to `Status` or lower. But we only want the stack trace to be included at the much lower `Trace` severity:

```c++
LOG(log_group, np::log::Status, "Sending message to server", WITH(msg), WITH(hostname), WITH(response_code), WITH(np::log::Trace, stacktrace, getStackTrace()));
```

Now, if nplog has been configured with a severity threshold of `Status`, this line will log the following:

`{"message":"Sending message to server","props":{"msg": "FOO","hostname":"google.com","response_code":200,"response":"OK"}}`

There's no stack trace, and crucially, the `getStackTrace()` function is never even called. Adding the property costs us nothing, because it's never evaluated.

But if we set the threshold to `Trace`, we might get this instead:

`{"message":"Sending message to server","props":{"msg": "FOO","hostname":"google.com","response_code":200,"response":"OK","stacktrace":"somefunc():42\nmain():15\n"}}`

And we get the stack trace we wanted!

### Separate severity thresholds for properties

This is all well and good, but maybe I want to enable *properties* at the `Trace` severity level, but I don't want to enable all the *messages* that may exist in the code with that severity.

Luckily, we can specify different thresholds for properties and messages with another call to `applyConfig()`:

```c++
    np::log::Config cfg;
    // Log messages should be skipped if their severity is below DebugHigh
    cfg.levels.default_level = {threshold(np::log::Status), threshold(np::log::Trace)};
    // apply the specified configuration
    np::log::applyConfig(cfg);
```

Now, messages will be logged if their severity level is `Status` or higher, but any properties they contain will be included if that property's severity is `Trace` or higher!

Most of the properties we've defined didn't have an explicit severity level. In those cases, they simply inherit the severity of the message they're part of.

This can be useful if you are troubleshooting an issue and you want to temporarily crank up the detail on your log messages and capture more state than you normally would. But it can also be used in the opposite way:

When your application is running normally, you generally don't want to log more than necessary, because excessive logging slows down your application. At the same time, you don't want to log too little, because if a problem occurs, you want to be able to see what happened! This is an uncomfortable tradeoff and most logging libraries offer no good solution.

But with nplog we can set the severity threshold for properties to be *higher* than normal too.

```c++
    np::log::Config cfg;
    // Log messages should be skipped if their severity is below DebugHigh
    cfg.levels.default_level = {threshold(np::log::Status), threshold(np::log::Fatal)};
    // apply the specified configuration
    np::log::applyConfig(cfg);
```

Now we'll still get all the messages at the `Status` level and higher, but their properties will be skipped, unless they explicitly have a `Fatal` severity.

Now, the same log statement as before will result in this message:

`{"message":"Sending message to server"}`

As you can imagine, this is much cheaper than having to evaluate each property. It still gives us an indication of the program flow, but it doesn't tell us the specific application state at the time. In many cases, you might find this to be a suitable default. You can allow most messages to be logged, but skip their properties, and avoid most of the performance overhead. Or you may wish to strike some other balance, perhaps saying that as long as Fatal properties are enabled, we want to see the hostname. If Status properties are enabled, we also want to see the response code and the message being sent, and only if Trace properties are enabled, do we want a stack trace from the call site:

```c++
LOG(log_group, np::log::Status, "Sending message to server", WITH(msg), WITH(np::log::Fatal, hostname), WITH(response_code), WITH(np::log::Trace, stacktrace, getStackTrace()));
```