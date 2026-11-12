// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/range_owning_param.c
// Pins: 0060 item 13: another count beside owning(n) on one parameter is rejected
// Pre-registered from record 0060 item 13 (Part E), before the code
#include <cerastes.h>
#include <stddef.h>

void f(int n, int *p CERASTES_OWNING_N(n))
    CERASTES_REQUIRES(cerastes_owns_range(p, n + 1))
{
  (void)n;
  (void)p;
}
