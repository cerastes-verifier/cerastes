// expect: error permission @ L11
// mode: c23, c++20
// twin: tests/memory/range_decreases_elem.c
// Pins: 0060 item 5 (0039 item 8): with n >= 0 only, the measure p[0] reads an element the range may not hold: permission at the decreases clause
// Pre-registered from record 0060 item 5, before the code
#include <cerastes.h>

int f(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n >= 0)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == n)
    CERASTES_DECREASES(p[0])
{
  (void)p;
  return n;
}
