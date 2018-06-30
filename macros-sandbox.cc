#include <experimental/optional>
#include <string>
#include <tuple>
#include <iostream>

template <typename T>
using optional = std::experimental::optional<T>;
extern void doIt(void*);

struct NoArg{};

bool test(int level);

struct ScopedMessage {
  ScopedMessage(std::string) {}

  void handleArg0() {}

  template <typename Head, typename ...Args>
  void handleArg0(std::tuple<int, const char*, Head> head, std::tuple<int, const char*, Args>... args) {
    if (test(std::get<0>(head))) {
        auto x = std::get<2>(head)();
        doIt(&x); }
    handleArg0(args...);
  }

  template <typename ...Args>
  ScopedMessage& operator()(std::tuple<int, const char*, Args>... args) {
      handleArg0(args...);
    return *this;
  }
  void handleArg1() {}
  template <typename Head, typename ...Args>
  void handleArg1(const char*, optional<Head> head, Args... args) {
    if (head) { doIt(&head); }
    handleArg1(args...);
  }

  template <typename ...Args>
  ScopedMessage& operator()(Args... args) {
      handleArg1(args...);
    return *this;
  }
//  template <typename Func>
//  ScopedMessage& arg(int level, const char* name, Func f) {
//    if (test(level)) { f(); }
//    return *this;
//  }

  template <typename T>
  ScopedMessage& arg(const char* name, T result) {
    doIt(&result);
    return *this;
  }

  ScopedMessage& arg(const char* name, NoArg) {
    return *this;
  }
};

template <int I>
struct Chatty {
    static inline int ctor = 0;
    static inline int dtor = 0;
    static inline int copyctor = 0;
    static inline int movector = 0;
    static inline int copyass = 0;
    static inline int moveass = 0;

    static void dump() {
        std::cout << "constructed: " << ctor << '\n'
            << "destroyed: " << dtor << '\n'
            << "copy constructed: " << copyctor << '\n'
            << "move constructed: " << movector << '\n'
            << "copy assigned: " << copyass << '\n'
            << "move assigned: " << moveass << std::endl;
    }

    Chatty() {++ctor; }
    ~Chatty() {++dtor; }
    Chatty(const Chatty&) {++copyctor; hello(); }
    Chatty(Chatty&&) {++movector; hello(); }
    Chatty& operator=(const Chatty&) { ++copyass; }
    Chatty& operator=(Chatty&&) { ++moveass; }

    void hello(){
        abort();
    }
};

int foo() { return 42; }

struct S0 {};
struct S1 {};
struct S2 {};
struct S3 {};
struct S4 {};
struct S5 {};
struct S6 {};
struct S7 {};
struct S8 {};
struct S9 {};
struct S10 {};
struct S11 {};
struct S12 {};
struct S13 {};
struct S14 {};
struct S15 {};
struct S16 {};
struct S17 {};
struct S18 {};
struct S19 {};
struct S20 {};
struct S21 {};
struct S22 {};
struct S23 {};
struct S24 {};
struct S25 {};
struct S26 {};
struct S27 {};
struct S28 {};
struct S29 {};


void burp() {
  //    LOG("hello", ARG(2+2), ARG(foo()));

#ifdef V1
  // Impl A
  {
    ScopedMessage msg({});
    if (test(1)) { msg.arg("2 + 2", 2 + 2); }
    if (test(2)) { msg.arg("foo()", foo()); }
  }
  {
    ScopedMessage msg({});
    if (test(1)) { msg.arg("S0", S0{}); }
  }

#define SHORT(N) \
  { ScopedMessage msg({}); if (test(1)) { msg.arg("S", S ## N{}); } }

#elif defined(BORKED)
  // Impl B: no-go: differing types for filtered-out args
  {
//      ScopedMessage({})
//      .arg("2 + 2", test(1) ? 2 + 2 : NoArg{})
//      .arg("foo()", test(2) ? foo() : NoArg{});
  }
  // Impl C
#elif defined(V0)
  {
    ScopedMessage({})(
      std::make_tuple(1, "2 + 2", [&]() { return 2 + 2; }), std::make_tuple(2, "foo()", [&]() { return foo(); }));
  }

#define SHORT(N) \
  ScopedMessage({})(std::make_tuple(1, "S", [&]() { return S ## N{}; }));

#elif defined(V2)
  {
    ScopedMessage({})("foo()", test(1) ? optional<decltype(foo())>(foo()) : optional<decltype(foo())>{}, "2+2", test(1) ? optional<decltype(2+2)>(2+2) : optional<decltype(2+2)>{});
  }

#define SHORT(N) \
  ScopedMessage({})( "S", test(1) ? optional<decltype(S ## N{})>(S ## N{}) : optional<decltype(S ## N{})>{})
#endif

#ifndef SAME
  SHORT(0);
  SHORT(1);
  SHORT(2);
  SHORT(3);
  SHORT(4);
  SHORT(5);
  SHORT(6);
  SHORT(7);
  SHORT(8);
  SHORT(9);
  SHORT(10);
  SHORT(11);
  SHORT(12);
  SHORT(13);
  SHORT(14);
  SHORT(15);
  SHORT(16);
  SHORT(17);
  SHORT(18);
  SHORT(19);
  SHORT(20);
  SHORT(21);
  SHORT(22);
  SHORT(23);
  SHORT(24);
  SHORT(25);
  SHORT(26);
  SHORT(27);
  SHORT(28);
  SHORT(29);

#else

  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
  SHORT(0);
#endif
}

#ifdef MAIN
int main() {
  {
    ScopedMessage({})(std::make_tuple(1, "S", [&]() { return Chatty<0>{}; }));

    {
      ScopedMessage msg({});
      if (test(1)) { msg.arg("S", Chatty<1>{}); }
    }

    ScopedMessage({})("S",
      test(1) ? optional<decltype(Chatty<2>{})>(Chatty<2>{}) : optional<decltype(Chatty<2>{})>{});
  }
  std::cout << "fun\n";
  Chatty<0>::dump();
  std::cout << "safe\n";
  Chatty<1>::dump();
  std::cout << "opt\n";
  Chatty<2>::dump();
}
#endif

#if 0
#ifdef V1
#define ARG(level, name, value) if (test(level)) { sm.arg(name, value); }
#define LOG(msg, ...) \
  { \
    ScopedMessage sm({}); \
    __VA_ARGS__ \
  }

#endif

#ifdef V0
#define LOG ScopedMessage({})
#endif
#endif

void flumph() {
    // fails because of comma between args. There might be some clever way to work around, but....
    //    LOG("hello", ARG(1, "2 + 2", 2 + 2), ARG(3, "foo()", foo()));

}

// ok, now ditch all the log specific stuff, and just try to set up the impls. Then we can mess with
// compile times and look at side effects more closely
#ifdef MAIN
void doIt(void*) {}
bool test(int) { return true; }
#endif
