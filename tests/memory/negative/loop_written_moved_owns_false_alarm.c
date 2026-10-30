// expect: error permission @ L27
// expect: error permission @ L27
// expect: error permission @ L28
// mode: c23, c++20
// twin: tests/memory/loop_written_two_atoms.c
// Pins: 0040 item 5 Amounts: a documented false alarm (audit round 2 S4): the liveness amount does not follow the place; lifted when it does (handoff question)
#include <cerastes.h>
#include <stddef.h>

void f(int *a, int *b, int n)
    CERASTES_REQUIRES(cerastes_owns(*a) && cerastes_owns(*b) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*a) && cerastes_owns(*b))
{
  int *t = a;
  a = t;
  int *q = a;
  int *r = b;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && ((q == a && r == b) || (q == b && r == a)) && cerastes_owns(*q) && cerastes_owns(*r))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int *s = q;
    q = r;
    r = s;
    *q = i;
    i += 1;
  }
}
