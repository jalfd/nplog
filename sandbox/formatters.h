#ifndef NP_LOG_FORMATTERS_HPP
#define NP_LOG_FORMATTERS_HPP
// ok, now...
// I  guess the two tasks are format and serialize
// format takes a UDT and stringifies it
// serialize generates the the output format (JSON or plaintext prettyprint)
// the basic signature of a format function is

namespace np {
    template <typename OutIter, typename ArgType>
    OutIter format(OutIter dest, ArgType arg);
}
#endif
