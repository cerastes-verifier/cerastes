// expect: error permission @ L20
// mode: c23, c++20
// twin: tests/memory/loop_return_owns.c
// Pins: 0038 item 3: the callee kept *p before the return inside the loop, so the return does not give back what its ensures promises
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int find_five(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && 0 <= r && r <= n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (i == 5) {
      dispose(p);
      return i;
    }
    i += 1;
  }
  return n;
}
