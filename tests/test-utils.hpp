#ifndef NP_LOG_TEST_UTILS_HPP
#define NP_LOG_TEST_UTILS_HPP

#include <iostream>

namespace {
  template <int I>
  struct Chatty {
    static inline int ctor = 0;
    static inline int dtor = 0;
    static inline int copyctor = 0;
    static inline int movector = 0;
    static inline int copyassign = 0;
    static inline int moveassign = 0;

    static void dump() {
      std::cout << "constructed: " << ctor << '\n'
                << "destroyed: " << dtor << '\n'
                << "copy constructed: " << copyctor << '\n'
                << "move constructed: " << movector << '\n'
                << "copy assigned: " << copyassign << '\n'
                << "move assigned: " << moveassign << std::endl;
    }

    Chatty() { ++ctor; }
    ~Chatty() { ++dtor; }
    Chatty(const Chatty&) { ++copyctor; }
    Chatty(Chatty&&) { ++movector; }
    Chatty& operator=(const Chatty&) { ++copyassign; }
    Chatty& operator=(Chatty&&) { ++moveassign; }
  };
} // namespace

#endif
