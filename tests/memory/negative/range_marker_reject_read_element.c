// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/range_marker_must_init.c
// Pins: 0060 item 12 (as 0042 item 5): a contract read p[e] of a marked parameter is rejected: its flag is not promised on entry
// Pre-registered from record 0060 item 12 (Part E), before the code
#include <cerastes.h>
#include <stddef.h>

void f(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns_range(p, 3) && p[0] > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, 3))
{
  (void)p;
}
