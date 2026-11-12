// expect: error check @ L22
// mode: c23, c++20
// twin: tests/memory/range_call_reads.c
// Pins: 0060 items 6 and 8: values are kept across a reads_range call (range_call_reads.c) and lost across an owns_range call: after zero_first(a, 3) the callee says nothing about a[1], so the check fails
// Pre-registered from record 0060 items 6 and 8 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

void zero_first(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[0] == 0)
{
  (void)n;
  p[0] = 0;
}

int lost(void)
    CERASTES_ENSURES(r, r == 2)
{
  int a[3] = {1, 2, 3};
  zero_first(a, 3);
  CERASTES_CHECK(a[1] == 2);
  return a[1];
}
