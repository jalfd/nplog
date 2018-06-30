#include <experimental/optional>
#include <string>
#include <iostream>

template <typename T>
using optional = std::experimental::optional<T>;
extern void doIt(const void*);

struct NoArg{};

bool test(int level);

struct ScopedMessage {
  ScopedMessage(std::string) {}

  void handleArg0() {}

  template <typename Head, typename ...Args>
  void handleArg0(int level, const char*, Head head, Args... args) {
      std::cout << "handlearg0\n";
      if (test(level)) {
        std::cout << "doing it\n";
        const auto &it = head();
        doIt(&it);
      }
      handleArg0(args...);
  }

  template <typename ...Args>
  ScopedMessage& var(Args... args) {
      std::cout << "var\n";
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
  ScopedMessage& opt(Args... args) {
      handleArg1(args...);
    return *this;
  }

  template <typename T>
  ScopedMessage& arg(const char* name, T result) {
    doIt(&result);
    return *this;
  }

  ScopedMessage& arg(const char* name, NoArg) {
    return *this;
  }

  using BufIter = char*;
  BufIter putIter() { return nullptr; }
  void putEnd(BufIter) {}
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

template <typename T>
bool serialize(ScopedMessage m, const char* name, const T& expr);
void test_instantiate() {
#if defined(V0)
  {
    ScopedMessage({}).var(
      1, "2 + 2", [&]() { return 2 + 2; }, 2, "foo()", [&]() { return foo(); });
  }

#define SHORT(N) \
  ScopedMessage({}).var(1, "S", [&]() { return S ## N{}; });

#elif defined(V1)
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

#elif defined(V2)
  {
    ScopedMessage({}).opt("foo()", test(1) ? optional<decltype(foo())>(foo()) : optional<decltype(foo())>{}, "2+2", test(1) ? optional<decltype(2+2)>(2+2) : optional<decltype(2+2)>{});
  }

#define SHORT(N) \
  ScopedMessage({}).opt( "S", test(1) ? optional<decltype(S ## N{})>(S ## N{}) : optional<decltype(S ## N{})>{})
#elif defined(V3)
  {
    ScopedMessage log({});
    (void)(test(1) ? serialize(log, "2 + 2", 2 + 2) : false), (test(1) ? serialize(log, "foo()", foo()) : false);
  }

#define SHORT(N) \
  { \
    ScopedMessage log({}); \
    (void)(test(1) ? serialize(log, "S", S ## N{}) : false); \
  }

//#error Integrate with the others
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
    ScopedMessage({}).var(1, "S", [&]() { return Chatty<0>{}; });

    {
      ScopedMessage msg({});
      if (test(1)) { msg.arg("S", Chatty<1>{}); }
    }

    ScopedMessage({}).opt("S",
      test(1) ? optional<decltype(Chatty<2>{})>(Chatty<2>{}) : optional<decltype(Chatty<2>{})>{});

    {
        ScopedMessage log({}); \
            (void)(test(1) ? serialize(log, "S", Chatty<3>{}) : false); \

    }
  }
  std::cout << "fun\n";
  Chatty<0>::dump();
  std::cout << "safe\n";
  Chatty<1>::dump();
  std::cout << "opt\n";
  Chatty<2>::dump();
  std::cout << "new\n";
  Chatty<3>::dump();
}
#endif

#if MACROSTEST
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

void flumph() {
    // fails because of comma between args. There might be some clever way to work around, but....
    //    LOG("hello", ARG(1, "2 + 2", 2 + 2), ARG(3, "foo()", foo()));

}
#endif

// ok, now ditch all the log specific stuff, and just try to set up the impls. Then we can mess with
// compile times and look at side effects more closely
#ifdef MAIN
void doIt(const void*) {}
bool test(int i) { return i < 5; }
#endif
