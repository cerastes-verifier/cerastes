// expect: error permission @ L20
// mode: c23, c++20
// twin: tests/memory/loop_calls.c
// Pins: 0038 item 4: a callee that consumes an outer cell: the iteration ends without the permission it began with, at the body's closing brace
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int consume_in_loop(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int x = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    dispose(&x);
    i += 1;
  }
  return i;
}
