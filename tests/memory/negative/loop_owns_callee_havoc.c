// expect: error postcondition @ L23
// mode: c23, c++20
// twin: tests/memory/loop_reads_callee.c
// Pins: 0038 item 3: a callee whose contract owns an int cell havocs every int place, so x loses its value
#include <cerastes.h>

void poke(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p));

int keep_after_poke(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 7)
{
  int x = 7;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    poke(&x);
    i += 1;
  }
  return x;
}
