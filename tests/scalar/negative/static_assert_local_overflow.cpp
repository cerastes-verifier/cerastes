// expect: error overflow @ L21
// twin: tests/scalar/static_assert_named.cpp
// Pins: 0024 item 2: static_assert_named.cpp with a local constexpr unsigned 65536u squared in the block assertion
#include <cerastes.h>

const int G = 1 << 3;
constexpr int K = G * 2;
const unsigned U = 65535u;
const signed char SC = (signed char)100;

static_assert(G == 8);
static_assert(K == 16, "a constant built from a constant");
static_assert(U * 65537u == 4294967295u, "an unsigned product of a named constant");
static_assert(SC == 100, "a narrowed constant");

int total(int x)
    CERASTES_REQUIRES(x >= 0 && x <= 100)
    CERASTES_ENSURES(r, r == x + 16 + 5)
{
  constexpr unsigned L = 65536u;
  static_assert(L * L == 0u, "a local constexpr that wraps");
  return x + K + 5;
}

int use(void)
    CERASTES_ENSURES(r, r == 121)
{
  return total(100);
}
