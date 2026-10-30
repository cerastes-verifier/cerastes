// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/loop_owns_fill.c
// Pins: 0038 item 4: a callee in the body consumes a parameter's place: the iteration ends without the permission it began with, reported at the body's closing brace
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

void drop_in_loop(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 100)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    dispose(p);
    i += 1;
  }
}
