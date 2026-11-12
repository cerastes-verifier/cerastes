// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/memory/range_get_set.c
// Pins: 0060 item 9: get claims r == 0 where the element is p[i]: the postcondition fails at the return, and nothing else (the range is given back)
// Pre-registered from record 0060 item 9, before the code
#include <cerastes.h>

int get(int *p, int n, int i)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= i && i < n)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == 0)
{
  (void)n;
  return p[i];
}

void set(int *p, int n, int i, int v)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= i && i < n)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[i] == v)
{
  (void)n;
  p[i] = v;
}
