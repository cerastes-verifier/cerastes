// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/range_get_set.c
// Pins: 0060 items 1 and 3 (M4.5e2): a pointer loaded from memory is not a base
// Pre-registered from record 0060 items 1 and 3, before the code
#include <cerastes.h>
#include <stddef.h>

void f(int **pp, int n)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns_range(*pp, n))
{
  (void)pp;
  (void)n;
}
