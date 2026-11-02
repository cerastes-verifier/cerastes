// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/memory/init_loop.c
// Pins: 0042 item 5: the only write is inside a loop that may not run: the flag row at the return fails
#include <cerastes.h>

void fill(int *p CERASTES_MUST_INIT, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p = i;
    i += 1;
  }
}  // <- here
