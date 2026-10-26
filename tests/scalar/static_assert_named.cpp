// expect: verified
// expect: native-ok
// Pins: 0024 item 2: a named constant in an assertion is read through the same path as a case value (C++ only: a const object is not a constant expression in C, #1901)
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
    CERASTES_REQUIRES(x <= 2147483626)
    CERASTES_ENSURES(r, r == x + 16 + 5)
{
  constexpr int L = 5;
  static_assert(L + G == 13, "a local constexpr and a global");
  return x + K + L;
}

int use(void)
    CERASTES_ENSURES(r, r == 2147483647)
{
  return total(2147483626);
}
