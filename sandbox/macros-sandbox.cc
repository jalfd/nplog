#include <string>
#include <iostream>

// just throw the version with main into ninja. We don't really need this file any more, but just to keep it alive for now
#define MAIN 1
struct ScopedMessage {
  ScopedMessage(std::string) {}

  using BufIter = char*;
  BufIter putIter() { return nullptr; }
  void putEnd(BufIter) {}

  // helpers to be used by args:
  
  bool testArg(int i); // test against log's settings
  bool testArg(const char* = nullptr); // assume arg uses the same debug level as the log message

  template <typename T>
      bool serialize(const char* name, const T& expr);

  const char* getName(const char* name, const char*) { return name; }
  const char* getName(int, const char* name) { return name; }

private:
  int level;
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
    Chatty(const Chatty&) {++copyctor; }
    Chatty(Chatty&&) {++movector; }
    Chatty& operator=(const Chatty&) { ++copyass; }
    Chatty& operator=(Chatty&&) { ++moveass; }
};

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

int foo() { return 42; }

void test_instantiate() {
#if defined(V3)
  {
    ScopedMessage sm({});
    (void)(sm.testArg(1) ? sm.serialize("2 + 2", 2 + 2) : false), (sm.testArg(1) ? sm.serialize("foo()", foo()) : false);
  }

#define SHORT(N) \
  { \
    ScopedMessage sm({}); \
    (void)(sm.testArg(1) ? sm.serialize("S", S ## N{}) : false); \
  }

#else
#define SHORT(N) \
  (void)S ## N{};
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
    ScopedMessage sm({});
    (void) (sm.testArg(1) ? sm.serialize("S", Chatty<3>{}) : false);
  }
  Chatty<3>::dump();
}
#endif

#if MACROSTEST

#define ARG3(arg0, arg1, arg2) ((sm.testArg(arg0)) ? sm.serialize(arg1, arg2) : false)
#define ARG2(arg0, arg1) ((sm.testArg(arg0)) ? sm.serialize(sm.getName(arg0, #arg1), arg1) : false)
#define ARG1(arg0) ((sm.testArg()) ? sm.serialize(#arg0, arg0) : false)

#define INTERNAL_NP_VAR_MACRO_SELECTOR(_1,_2,_3,NAME,...) NAME
#define ARG(...) INTERNAL_NP_VAR_MACRO_SELECTOR(__VA_ARGS__, ARG3, ARG2, ARG1)(__VA_ARGS__)


//#define ARG(level, name, value) ((test(level)) ? serialize(sm, name, value) : false)
#define LOG(msg, ...) \
  { \
    ScopedMessage sm({}); \
    (void) __VA_ARGS__ ; \
  }

int bar(int x, int y) { return x + y}
void macros() {
    LOG("hello", ARG(1, "2 + 2", 2 + 2), ARG(3, "foo()", foo()), ARG(3, "bar()", bar(3,4)), ARG(3, bar(3,4)), ARG("name", bar(3,4)));
}
#endif

#ifdef MAIN
bool ScopedMessage::testArg(int i) { return i < 5; }
bool ScopedMessage::testArg(const char*) { return testArg(level);} // assume same debug level as the log message

template <typename T>
bool ScopedMessage::serialize(const char* name, const T& expr){ return true;}
#endif

