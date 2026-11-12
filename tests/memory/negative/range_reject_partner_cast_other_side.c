// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/range_partners.c
// Pins: 0060 item 3: the cast in the requires only ((long)n against n): rejected at the ensures atom
// Pre-registered from record 0060 items 1 and 3, before the code
#include <cerastes.h>

int f(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, (long)n))
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == n)
{
  (void)p;
  return n;
}
