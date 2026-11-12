// expect: error permission @ L21
// mode: c23, c++20
// twin: tests/memory/range_call_separation.c
// Pins: 0060 item 8 (audit round 3, S1, s1): f(&g, 1) hands the global as the range and the callee also owns it as a cell: the separation row of a range and a global cell fails at the call: permission, not an exit 3
// Pre-registered from record 0060 item 8 (audit round 3), before the code
#include <cerastes.h>
#include <stddef.h>

int g;

void f(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_owns(g))
{
  (void)p;
  (void)n;
}

int main(void)
{
  f(&g, 1);
  return 0;
}
