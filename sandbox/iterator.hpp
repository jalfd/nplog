#ifndef NP_ITERATOR_HPP
#define NP_ITERATOR_HPP

namespace np {
    template <typename OutIter, typename DerivedIter>
    struct TransformOutIterator
    {
    public:
        using value_type = typename DerivedIter::value_type;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::output_iterator_tag;

        TransformOutIterator(OutIter it) : it(it) {}

// FIXME
        value_type operator*() {
          return value_type([this](char c) { return *it++ = c; });
        }

        // post-increment
        TransformOutIterator &operator++(int)
        {
            auto tmp = *this;
            return ++tmp;
        }

        // pre-increment 
        TransformOutIterator& operator++()
        {
            ++it;
            return *this;
        }

        const OutIter& wrapped() const {
            return it;
        }

    protected:

        OutIter it;
    };

    template <typename T, typename F>
    bool operator==(const TransformOutIterator<T, F> &lhs,
                    const TransformOutIterator<T, F> &rhs) {
      return lhs.wrapped() == rhs.wrapped();
    }

    template <typename T, typename F>
    bool operator!=(const TransformOutIterator<T, F> &lhs,
                    const TransformOutIterator<T, F> &rhs) {
      return !(lhs == rhs);
    }


    template <typename OutIter>
    struct JsonOutIterator : TransformOutIterator<OutIter, JsonOutIterator<OutIter>>
    {
        struct ValueWrapper {
            explicit ValueWrapper(OutIter it) {}
            void operator=(char c) {
                if (c < 0x20 || c == '"' || c == '\\') {
                    *it++ = '\\';
                    switch (c) {
                        case '"':
                            *it++ = '"';
                        case '\\':
                            *it++ = '\\';
                        case '\n':
                            *it++ = 'n';
                        case '\r':
                            *it++ = 'r';
                        case '\t':
                            *it++ = 't';
                        default:
                        {
                            *it++ = 'u';
                            *it++ = '0';
                            *it++ = (c > 0xf ? '1' : '0');
                            char tmp = c & 0xf;
                            *it++ = tmp > 9 ? ('a' + (tmp-10)) : '0' + tmp;
                        }
                    }
                    return;
                }
                *it++ = c;
            }
            OutIter it;
        };
        using value_type = ValueWrapper;
    };
}

#endif
