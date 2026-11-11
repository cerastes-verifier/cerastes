// expect: error permission @ L22
// mode: c23, c++20
// twin: tests/memory/array_loop_callee.c
// Pins: 0059 item 8: a callee keeps a[1] in the body: the iteration ends without the permission it began with, a marked back-edge row reported at the body's closing brace
// Pre-registered from record 0059 item 8 (Part C), before the code
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int run(int n)
    CERASTES_REQUIRES(0 <= n && n <= 3)
    CERASTES_ENSURES(r, r == n)
{
  int a[3] = {1, 2, 3};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    dispose(&a[1]);
    i = i + 1;
  }
  return i;
}
