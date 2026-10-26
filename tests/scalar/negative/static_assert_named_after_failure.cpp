// expect: error conversion @ L9
// twin: tests/scalar/static_assert_named.cpp
// Pins: 0024 item 2: static_assert_named.cpp with SC = (signed char)300 (44): the constant fails once at its declaration; the assertion that reads it reports nothing more
#include <cerastes.h>

const int G = 1 << 3;
constexpr int K = G * 2;
const unsigned U = 65535u;
const signed char SC = (signed char)300;

static_assert(G == 8);
static_assert(K == 16, "a constant built from a constant");
static_assert(U * 65537u == 4294967295u, "an unsigned product of a named constant");
static_assert(SC == 44, "a narrowed constant");

int total(int x)
    CERASTES_REQUIRES(x >= 0 && x <= 100)
    CERASTES_ENSURES(r, r == x + 16 + 5)
{
  constexpr int L = 5;
  static_assert(L + G == 13, "a local constexpr and a global");
  return x + K + L;
}

int use(void)
    CERASTES_ENSURES(r, r == 121)
{
  return total(100);
}
