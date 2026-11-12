// expect: error out-of-bounds @ L14
// expect: error permission @ L14
// mode: c23, c++20
// twin: tests/memory/range_index_bound.c
// Pins: 0060 item 10 (S43 N1): i <= n lets i == n: the bound row and the slot's permission each fail at the read, two real cases at one line
// Pre-registered from record 0060 item 10, before the code
#include <cerastes.h>

int first(int *p, int n, int i)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= i && i <= n)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n))
{
  (void)n;
  return p[i];
}

void clear(int *p, int n, int i)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= i && i < n)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  (void)n;
  p[i] = 0;
}
