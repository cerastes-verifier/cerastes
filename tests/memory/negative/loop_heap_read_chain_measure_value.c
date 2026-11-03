// expect: error termination @ L21
// mode: c23, c++20
// twin: tests/memory/loop_heap_read_chain_measure.c
// Pins: 0044 item 12 (P69): the measure p->q->x is constant (the cell never changes): termination at the clause
#include <cerastes.h>

struct L {
  struct L *q;
  int x;
};

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 10)
    CERASTES_ENSURES(r, r >= 0)
{
  struct L t = {0, n};
  struct L s = {&t, 0};
  struct L *p = &s;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p == &s && p->q == &t && t.x == n)
  CERASTES_DECREASES(p->q->x)  // <- here
  while (i < n) {
    i += 1;
  }
  return p == &s ? t.x : 0;
}
