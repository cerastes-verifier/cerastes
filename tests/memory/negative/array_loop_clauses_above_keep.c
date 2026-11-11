// expect: error permission @ L22
// mode: c23, c++20
// twin: tests/memory/array_loop_clauses_above.c
// Pins: 0059 item 8: a callee kept a[2] before the loop; the clauses are on lines of their own above the while, and the entry row is reported at the while's own line in both modes (the keyword, as missing_decreases is)
// Pre-registered from record 0059 item 8 (Part C): the marked entry row at the while
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int run(int n)
    CERASTES_REQUIRES(0 <= n && n <= 3)
    CERASTES_ENSURES(r, r == n)
{
  int a[3] = {1, 2, 3};
  int i = 0;
  int s = 0;
  dispose(&a[2]);
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_INVARIANT(s == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s = s + a[0] - a[0] + 1;
    i = i + 1;
  }
  return s;
}
