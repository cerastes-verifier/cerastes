// expect: rejected cerastes-missing_decreases @ L8
// mode: c23, c++20
// twin: tests/scalar/pure_tri.c
// Pins: 0062 item 10: a pure function that calls itself needs a decreases (missing_decreases, reported at its definition as 0013 item 4 has it)
// Pre-registered from record 0062 (Part B), before the code: rejected
#include <cerastes.h>

CERASTES_PURE int tri(int n)
    CERASTES_REQUIRES(0 <= n && n <= 1000)
    CERASTES_ENSURES(r, 0 <= r && r <= 1000 * n)
{
  return n == 0 ? 0 : tri(n - 1) + n;
}

int use(void)
{
  CERASTES_CHECK(tri(0) == 0);
  return 0;
}
