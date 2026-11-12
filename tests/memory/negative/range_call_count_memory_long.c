// expect: error permission @ L22
// mode: c23, c++20
// twin: tests/memory/range_call_count_memory.c
// Pins: 0060 item 8 (audit S4): the same call with *q == 5 on a four-element array: the shape row fails at the call (the count is the local's value)
// Pre-registered from record 0060 item 8 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

void zero_first(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[0] == 0)
{
  (void)n;
  p[0] = 0;
}

int use(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q) && *q == 5)
    CERASTES_ENSURES(r, cerastes_owns(*q) && r == 0)
{
  int a[4] = {1, 2, 3, 4};
  zero_first(a, *q);
  return a[0];
}
