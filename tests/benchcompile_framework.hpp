#ifndef NP_BUILD_BENCH_FRAMEWORK_HPP
#define NP_BUILD_BENCH_FRAMEWORK_HPP
#define REPEAT0(N, EXPR) EXPR(N)
#define REPEAT1(N, EXPR) REPEAT0((N * 2), EXPR) REPEAT0((N * 2 + 1), EXPR)
#define REPEAT2(N, EXPR) REPEAT1((N * 2), EXPR) REPEAT1((N * 2 + 1), EXPR)
#define REPEAT3(N, EXPR) REPEAT2((N * 2), EXPR) REPEAT2((N * 2 + 1), EXPR)
#define REPEAT4(N, EXPR) REPEAT3((N * 2), EXPR) REPEAT3((N * 2 + 1), EXPR)
#define REPEAT5(N, EXPR) REPEAT4((N * 2), EXPR) REPEAT4((N * 2 + 1), EXPR)
#define REPEAT6(N, EXPR) REPEAT5((N * 2), EXPR) REPEAT5((N * 2 + 1), EXPR)
#define REPEAT7(N, EXPR) REPEAT6((N * 2), EXPR) REPEAT6((N * 2 + 1), EXPR)
#define REPEAT8(N, EXPR) REPEAT7((N * 2), EXPR) REPEAT7((N * 2 + 1), EXPR)
#define REPEAT9(N, EXPR) REPEAT8((N * 2), EXPR) REPEAT8((N * 2 + 1), EXPR)
#define REPEAT10(N, EXPR) REPEAT9((N * 2), EXPR) REPEAT9((N * 2 + 1), EXPR)

#define REPEAT_EXP(N, EXPR) REPEAT##N(0, EXPR)

namespace util {
  constexpr int pow(int i, int e) {
    if (e == 0) { return 0; }
    if (e == 1) { return i; }
    return i * pow(i, e - 1);
  }
} // namespace util

template <int N>
void func();

#define PRE_TEST(N) \
  template <> \
  void func<N>() {
#define POST_TEST(N) }

#if defined(SINGLE_TYPE)
// Repeat use of the same type
#define TEST_INSTANCE(N) PRE_TEST(N) TEST_IMPL(PARAMETER_TYPE(0)) POST_TEST(N)
#define INSTANTIATE_TYPE(N) template struct PARAMETER_TYPE(0);
#elif defined(CYCLE_TYPES)
// Cycle through types
#define TEST_INSTANCE(N) PRE_TEST(N) TEST_IMPL(PARAMETER_TYPE(N)) POST_TEST(N)
#define INSTANTIATE_TYPE(N) template struct PARAMETER_TYPE(N);
#elif defined(NULL_TEST)
// Null test
#define TEST_INSTANCE(N) PRE_TEST(N) POST_TEST(N)
#define INSTANTIATE_TYPE(N)
#else
#error Boo
#endif

#define CALL_STATIC_SETUP_PER_INSTANCE(N) STATIC_SETUP_PER_INSTANCE(PARAMETER_TYPE(N))

// Repeat use of the same type
// Cycle through types
// Null test

#define INSTANTIATE_BENCHMARK() \
  REPEAT_EXP(10, INSTANTIATE_TYPE) \
  STATIC_SETUP_ONCE() \
  REPEAT_EXP(10, CALL_STATIC_SETUP_PER_INSTANCE) \
  REPEAT_EXP(10, TEST_INSTANCE) \
  template <std::size_t... I> \
  void call2(std::index_sequence<I...>) { \
    (func<I>(), ...); \
  } \
  void call() { call2(std::make_index_sequence<util::pow(2, 10)>()); }
#endif
