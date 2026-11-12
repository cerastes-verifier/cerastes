// expect: error permission @ L21
// mode: c23, c++20
// twin: tests/memory/range_call_separation.c
// Pins: 0060 item 8 (audit round 3, S1, s4): an owned range with a reads cell of the same global: the pair has an owns, so the separation row applies and fails at the call: permission
// Pre-registered from record 0060 item 8 (audit round 3), before the code
#include <cerastes.h>
#include <stddef.h>

int g;

void f(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_reads(g))
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_reads(g))
{
  (void)p;
  (void)n;
}

int main(void)
{
  f(&g, 1);
  return 0;
}
