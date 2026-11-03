// expect: error permission @ L17
// mode: c23, c++20
// twin: tests/memory/loop_heap_read_ptrmember_measure.c
// Pins: 0044 item 12 (P69): the measure reads *p->q, which this loop does not hold: permission at the clause
#include <cerastes.h>

struct P {
  int *q;
};

int f(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->q != 0 && 0 <= n && n <= 10)
    CERASTES_ENSURES(r, r == 0 && cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i + *p->q * 0)  // <- here
  while (i < n) {
    i += 1;
  }
  return 0;
}
