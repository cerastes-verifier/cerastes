// expect: error postcondition @ L22
// mode: c23, c++20
// twin: tests/memory/range_get_set.c
// Pins: 0060 item 9: set claims p[i] == 0 where it stored v: the postcondition fails at the closing brace of the void function
// Pre-registered from record 0060 item 9, before the code
#include <cerastes.h>

int get(int *p, int n, int i)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= i && i < n)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == p[i])
{
  (void)n;
  return p[i];
}

void set(int *p, int n, int i, int v)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= i && i < n)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[i] == 0)
{
  (void)n;
  p[i] = v;
}
