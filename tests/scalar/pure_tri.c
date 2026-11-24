// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 items 9 and 10: a recursive pure function with a measure and a bound ensures: the function is checked by induction, its termination by Silicon, and its applications are used
// Pre-registered from record 0062 (Part B), before the code: verifies
#include <cerastes.h>

CERASTES_PURE int tri(int n)
    CERASTES_REQUIRES(0 <= n && n <= 1000)
    CERASTES_ENSURES(r, 0 <= r && r <= 1000 * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : tri(n - 1) + n;
}

int use(void)
{
  CERASTES_CHECK(tri(0) == 0);
  CERASTES_CHECK(tri(3) <= 3000);
  return 0;
}
