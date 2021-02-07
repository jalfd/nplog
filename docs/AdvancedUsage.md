# Advanced message filtering

In the [Getting Started guide](GettingStarted.md), we saw the `applyConfig()` function used to set a default severity threshold for messages and properties.

We also created a `LogGroup` object that all log statements referenced, but we didn't really cover what purpose it served.

Let's dig into that.

As said previously, different projects might have different requirements for controlling which messages should be logged, and which should be suppressed when the program runs.

The most common case is what we already covered: a single global severity threshold: all messages whose severity is at least this high get logged. Others are skipped.

## Message Filtering by Log Group Name
Some allow for more granularity by using separate loggers or named sections. This allows for certain messages, in a certain part of the code, to be tested against a separate threshold. That is, "in general, we may want to log all messages with severity `Status` or higher, but right now, I'm debugging an issue with the networking subsystem, so I'd like to crank up the severity threshold for *those* messages allowing them to be logged if their severity is `DebugMid` or higher"

Nplog supports this use case by assigning names to `LogGroups`:

```c++
np::LogGroup general_log;
np::LogGroup network_log("network");

…
LOG(general_log, np::log::Status, "This message is sent to the general log group");
LOG(network_log, np::log::DebugHigh, "This message is sent to the networking log group")
```

If nplog was configured with a default severity threshold of `Status`, then the first message will be logged, but the second will not. That's useful because when the application is running normally, you don't want all the detailed chatter from each component and subsystem.

But if a problem then occurs in the networking subsystem, we might want to tweak the severity level so that *globally* we still only want messages whose severity is at least `Status`, but messages logged against the network-related `LogGroup` should be logged if their severity is `DebugLow` or higher (nplog has three Debug severities, in decreasing order of severity: DebugHigh, DebugMid, DebugLow). To achieve that, we need to apply a new configuration:

```c++
np::log::Config cfg;
// Log messages should be skipped if their severity is below Status
cfg.levels.default_level = {threshold(np::log::Status)};
// ... unless they come from the network LogGroup, in which case they should only be skipped if they're below DebugLow!
cfg.levels.levels_by_name["network"] = {threshold(np::log::DebugLow)};

// apply the specified configuration
np::log::applyConfig(cfg);
```

Now, if we run our application, both messages will be logged.

## Message Filtering by Log Group Depth
Other applications may wish to filter log messages in a more generic and systematic way. Above, we distinguished between "general log messages", and "log messages from this specific named subsystem". We can generalize that, and build a hierarchy of LogGroups by assigning a parent to each.

Then we can specify severity thresholds based on how deep in the hierarchy a message's associated `LogGroup` is. If it uses the top-level `LogGroup` then we assume the message is of general interest, and we want to log it if its severity level is `DebugHigh` or higher. We can afford to be a bit chatty, because we only want to apply this to general messages about the application state as a whole.
Messages from subsystems on the other hand might contain lots of diagnostic data that we're just not generally interested in, so those should only be logged if their severity is at least `Status`. Finally, each subsystem might have even more specialized messages describing very specialized tasks, and those should only be logged if an error occurs.

If we create our `LogGroup` objects in a hierarchy of three levels, we can achieve this:

```c++
np::LogGroup base;
np::LogGroup subsystem(&base);
np::LogGroup specialized(&subsystem);
```

and then configure nplog to implement the above strategy:

```c++
np::log::Config cfg;
// If a message's severity is at least Error, it should *always* be logged
cfg.levels.default_level = {threshold(np::log::Error)};
// but if it is sent to the root LogGroup (depth 0), we also want messages of severity DebugHigh and up
cfg.levels.levels_by_depth[0] = {threshold(np::log::DebugHigh)};
//a and if it is sent to one of the subsystem LogGroups (depth 1), we want messages of severity Status and up
cfg.levels.levels_by_depth[1] = {threshold(np::log::Status)};

// apply the specified configuration
np::log::applyConfig(cfg);
```

Note that the depths specified are maximums. That is, `cfg.levels.levels_by_depth[3] = {threshold(np::log::Status)};` will ensure that messages of severity `Status` and up will *always* be logged if the log group is at most four levels deep in the hierarchy.

## Combining depth and name filtering
You can even merge the above strategies if you want to. When a message is logged, all applicable rules are gathered, and the message's severity is tested against the union of these rules. If just one rule would allow the message, the message is logged.

## Filtering sensitive properties
Some applications may have access to sensitive data that should normally never be logged.

And yet, when a problem occurs, you *do* want to be able to log it.

Examples of this may be any kind of personal information protected by the GDPR, patient health information for a health care application, or a password.