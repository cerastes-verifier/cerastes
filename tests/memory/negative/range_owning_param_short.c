// expect: error permission @ L35
// mode: c23, c++20
// twin: tests/memory/range_owning_param.c
// Pins: 0060 item 13: the call says the array has four elements; owning(n) takes owns_range(p, 4) and the caller holds three: the shape row fails at the call (permission), once, and the path ends
// Pre-registered from record 0060 item 13 (Part E), before the code
#include <cerastes.h>
#include <stddef.h>

void zero_first(int n, int *p CERASTES_OWNING_N(n))
{
  if (p != NULL && n > 0) {
    p[0] = 0;
  }
}

void zero_second(int n, CERASTES_OWNING_N(n) int *q)
{
  if (q != NULL && n > 1) {
    q[1] = 0;
  }
}

void merged(int n, int *p CERASTES_OWNING_N(n))
    CERASTES_REQUIRES(p == NULL || cerastes_owns_range(p, n))
    CERASTES_ENSURES(p == NULL || cerastes_owns_range(p, n))
{
  (void)n;
  (void)p;
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  int a[3] = {1, 2, 3};
  zero_first(4, a);
  zero_second(3, a);
  zero_first(0, NULL);
  merged(3, a);
  return 0;
}
