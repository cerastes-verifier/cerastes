// expect: error out-of-bounds @ L22
// expect: error permission @ L22
// mode: c23, c++20
// twin: tests/memory/range_index_bound.c
// Pins: 0060 item 10 (S43 N1): without 0 <= i the store may fall below the range: the bound row and the slot's write permission each fail at the store
// Pre-registered from record 0060 item 10, before the code
#include <cerastes.h>

int first(int *p, int n, int i)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= i && i < n)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n))
{
  (void)n;
  return p[i];
}

void clear(int *p, int n, int i)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && i < n)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  (void)n;
  p[i] = 0;
}
