// expect: error termination @ L20
// mode: c23, c++20
// twin: tests/memory/loop_heap_read_ptrmember_measure.c
// Pins: 0044 item 12 (P69): the measure *p->q is constant (the cell never changes): termination at the clause
#include <cerastes.h>

struct P {
  int *q;
};

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 10)
    CERASTES_ENSURES(r, r >= 0)
{
  int x = n;
  struct P s = {&x};
  struct P *p = &s;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p == &s && p->q == &x && x == n)
  CERASTES_DECREASES(*p->q)  // <- here
  while (i < n) {
    i += 1;
  }
  return p == &s ? x : 0;
}
