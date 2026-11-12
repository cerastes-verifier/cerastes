// expect: rejected cerastes-unsupported_annotation @ L11
// mode: c23, c++20
// twin: tests/memory/range_get_set.c
// Pins: 0060 items 1 and 3 (M4.5e2): an ensures range with no requires partner is rejected at the ensures atom
// Pre-registered from record 0060 items 1 and 3, before the code
#include <cerastes.h>
#include <stddef.h>

void f(int *p, int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  (void)p;
  (void)n;
}
