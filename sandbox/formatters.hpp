#ifndef NP_FORMATTERS_HPP
#define NP_FORMATTERS_HPP

namespace np {
    namespace internal {
        static struct ScopedLocale {
#ifdef _WIN32
                using locale_t = _locale_t;
#endif
                ScopedLocale() :
#ifdef _WIN32
                loc(_create_locale(LC_ALL, "C"))
#else
                loc(newlocale(LC_ALL_MASK, "C", 0))
#endif
                {}

                ~ScopedLocale()
                {
#ifdef _WIN32
                    _free_locale(loc);
#else
                    freelocale(loc);
#endif
                }
                locale_t loc;
                ScopedLocale(const ScopedLocale&) = delete;
                ScopedLocale& operator=(const ScopedLocale&) = delete;
        } c_locale;
    }

    template <typename OutIter, typename T>
    auto logStringify(OutIter it, const T& val) -> typename std::enable_if<!std::is_floating_point<T>::value, OutIter>::type
    {
        //    str_out_iter logStringify(str_out_iter it, const T& val) {
#ifdef FMT_VERSION
        return fmt::format_to(it, "{}", val);
#else
        const auto str = (std::ostringstream() << val).str();
        return std::copy(str.begin(), str.end(), it);
#endif
    }

        template <typename OutIter, typename T>
        auto logStringify(OutIter it, const T& val) -> typename std::enable_if<std::is_floating_point<T>::value, OutIter>::type
        {
            char buf[32];
#ifdef _WIN32
            const auto len = _snprintf_l(buf, 32, "%f", val, internal::c_locale.loc); // WIN32
#elif defined(__linux__)
            auto old_loc = uselocale(c_locale.loc);
            const auto len = snprintf(buf, 32, "%f", val);
            uselocale(old_loc);
#else
            const auto len = snprintf_l(buf, 32, internal::c_locale.loc, "%f", val);
#endif
            return std::copy_n(buf, len, it);
        }
}
#endif
