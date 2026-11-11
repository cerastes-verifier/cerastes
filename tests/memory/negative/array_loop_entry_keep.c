// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/array_loop_callee.c
// Pins: 0059 item 8: a callee kept a[2] before the loop, whose body reads a[0] (a loop that does not touch the array holds no range place): the loop begins without the permission to every element of a, a marked entry row reported at the loop; the clauses stand on the while's line because C++ reports a statement with attributes at its first attribute and C at the while (a test with the clauses on lines of their own would need one line per mode)
// Pre-registered from record 0059 item 8 (Part C), before the code
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int run(int n)
    CERASTES_REQUIRES(0 <= n && n <= 2)
    CERASTES_ENSURES(r, r == n)
{
  int a[3] = {1, 2, 3};
  int i = 0;
  int s = 0;
  dispose(&a[2]);
  CERASTES_INVARIANT(0 <= i && i <= n) CERASTES_INVARIANT(s == i) CERASTES_DECREASES(n - i) while (i < n) {
    s = s + a[0];
    i = i + 1;
  }
  return s;
}
