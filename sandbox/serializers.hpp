#ifndef NP_LOG_COMPOSERS_HPP
#define NP_LOG_COMPOSERS_HPP

#include <type_traits>
namespace np {
    namespace internal {
        template <typename T, typename Enable = void>
        struct is_numerical : std::false_type {};

        template <typename T>
        struct is_numerical<T, std::enable_if_t<std::is_arithmetic<T>::value>> : std::true_type {};

        template <> struct is_numerical<char> : std::false_type {};
        template <> struct is_numerical<wchar_t> : std::false_type {};
        template <> struct is_numerical<char16_t> : std::false_type {};
        template <> struct is_numerical<char32_t> : std::false_type {};
    }

    template <typename T>
    struct ScopedComposer
    {
        ScopedComposer(T& composer) : composer(composer) {}
        ~ScopedComposer() {
            composer.end();
        }

        auto outIterator();
        T& composer;
    };

    // A composer takes string data and composes it as the desired output format
    struct TextComposer {
        template <typename T, typename Enable = void>
        struct is_literal : std::false_type {};

        // no wait. The correct interface is "I want to create a [!]literal, give me an iterator
        // how do you tell it when you're done?
        // could return a scoped object which contains an iter
        // or you could just build a scopedcomposer
        template <typename OutIter, typename InIter>
        OutIter compose(OutIter dest, InIter first, InIter last, bool literal);
        // define an iterator type
    };

    struct JsonComposer {
        template <typename T, typename Enable = void>
        struct is_literal : std::false_type {};

        template <typename OutIter, typename InIter>
        OutIter compose(OutIter dest, InIter src, bool literal);
    };

    template <typename T>
    struct JsonComposer::is_literal<T, std::enable_if_t<internal::is_numerical<T>::value>> : std::true_type {};
}
#endif
