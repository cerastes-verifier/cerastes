// expect: error termination @ L13
// mode: c23, c++20
// twin: tests/scalar/pure_tri.c
// Pins: 0062 item 10: a measure that may be negative (n - 5 for n below 5): the termination error, at the recursive call
// Pre-registered from record 0062 (Part B), before the code: termination
#include <cerastes.h>

CERASTES_PURE int tri(int n)
    CERASTES_REQUIRES(0 <= n && n <= 1000)
    CERASTES_ENSURES(r, 0 <= r && r <= 1000 * n)
    CERASTES_DECREASES(n - 5)
{
  return n == 0 ? 0 : tri(n - 1) + n;
}

int use(void)
{
  CERASTES_CHECK(tri(0) == 0);
  CERASTES_CHECK(tri(3) <= 3000);
  return 0;
}
