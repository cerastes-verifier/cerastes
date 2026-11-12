// expect: error out-of-bounds @ L16
// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/range_loop_fill.c
// Pins: 0060 items 10 and 11 (S44 (g) toofar, N1): the loop runs to i <= n: p[n] fails its bound row and its write-permission row at the store (two real cases at one line; the failed permission row's assumption ends the path, so nothing is reported for the invariant)
// Pre-registered from record 0060 item 11 (Part D), before the code
#include <cerastes.h>
#include <stddef.h>

void fill(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n) CERASTES_DECREASES(n - i) while (i <= n) {
    p[i] = i;
    i++;
  }
}
