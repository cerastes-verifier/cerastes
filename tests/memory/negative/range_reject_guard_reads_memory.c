// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/range_get_set.c
// Pins: 0060 items 3 and 4 (audit round 2, S2): the guard of a range atom reads memory (*q > 0 ||): rejected (the marked rows' row failure must be its own, never a read in the guard); cell atoms accept heap-reading guards (0045 item 3), range atoms do not
// Pre-registered from record 0060 items 3 and 4 (audit round 2), before the code
#include <cerastes.h>
#include <stddef.h>

void f(int *p, int n, int *q)
    CERASTES_REQUIRES(cerastes_owns(*q) && (*q > 0 || cerastes_owns_range(p, n)))
    CERASTES_ENSURES(cerastes_owns(*q))
{
  (void)p;
  (void)n;
  (void)q;
}
