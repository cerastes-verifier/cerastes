// expect: error invariant-preserved @ L20
// mode: c23, c++20
// twin: tests/memory/loop_heap_read_chain_invariant.c
// Pins: 0044 item 12 (P69): the invariant claims p->q->x == 0, the body increments the cell: invariant-preserved at the clause
#include <cerastes.h>

struct L {
  struct L *q;
  int x;
};

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 10)
    CERASTES_ENSURES(r, r >= 0)
{
  struct L t = {0, 0};
  struct L s = {&t, 0};
  struct L *p = &s;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p == &s && p->q == &t && p->q->x == 0)  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    t.x += 1;
    i += 1;
  }
  return p == &s ? t.x : 0;
}
