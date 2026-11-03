// expect: error invariant-preserved @ L19
// mode: c23, c++20
// twin: tests/memory/loop_heap_read_ptrmember_invariant.c
// Pins: 0044 item 12 (P69): the invariant claims *p->q == 0, the body increments the cell: invariant-preserved at the clause
#include <cerastes.h>

struct P {
  int *q;
};

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 10)
    CERASTES_ENSURES(r, r >= 0)
{
  int x = 0;
  struct P s = {&x};
  struct P *p = &s;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p == &s && p->q == &x && *p->q == 0)  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    x += 1;
    i += 1;
  }
  return p == &s ? x : 0;
}
