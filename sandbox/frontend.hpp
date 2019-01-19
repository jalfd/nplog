#ifndef NP_LOG_FRONTEND_HPP
#define NP_LOG_FRONTEND_HPP

#include "common.hpp"
#include "dummy.hpp"

namespace np::log {
  template <typename ComposerType>
  struct Log {
      Log(ComposerType composer) : composer(composer) {}
      template <typename ...Args>
    void write(Header h, std::string_view msg, std::tuple<Severity, std::string_view, const Args*>... args) {
      if (h.severity < min_level) { return; }
      composer.write(h,
        msg,
        std::tuple<bool, std::string_view, const Args*>(
          std::get<0>(args) >= min_arg_level, std::get<1>(args), std::get<2>(args))...);
    }

    void setSeverity(Severity messages, Severity args) {
      min_level = messages;
      min_arg_level = args;
      }

    void setSeverity(Severity messages) {
      setSeverity(messages, messages);
    }

  private:
    ComposerType composer;
    Severity min_level = Severity::All;
    Severity min_arg_level = Severity::All;
  };

  template <typename LogType>
  struct ScopedMessage {
    ScopedMessage(LogType& log, Header hdr) : hdr(hdr), log(log) {}

    template <size_t LiteralLen, typename ...Args>
    void operator()(const char (&str)[LiteralLen],  std::tuple<Severity, std::string_view, const Args*>... args)
    {
        log.write(hdr, std::string_view(str, LiteralLen-1), args...);
    }

  private:
    Header hdr;
    LogType& log;
  };

  template <typename LogType>
  ScopedMessage<LogType> scopedMessage(LogType& log, Header hdr) {
    return ScopedMessage<LogType>(log, hdr);
  }
} // namespace np::log

#endif
