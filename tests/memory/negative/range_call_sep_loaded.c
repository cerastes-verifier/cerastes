// expect: error permission @ L22
// mode: c23, c++20
// twin: tests/memory/range_call_separation.c
// Pins: 0060 item 8 (audit round 3, S1, s2): **pp is a cell inside q's range: the separation row of a range and a cell reached through a load fails at the call: permission
// Pre-registered from record 0060 item 8 (audit round 3), before the code
#include <cerastes.h>
#include <stddef.h>

void f(int **pp, int *q, int n)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp) && cerastes_owns_range(q, n))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns(**pp) && cerastes_owns_range(q, n))
{
  (void)pp;
  (void)q;
  (void)n;
}

int main(void)
{
  int a[2] = {1, 2};
  int *x = &a[0];
  f(&x, a, 2);
  return 0;
}
