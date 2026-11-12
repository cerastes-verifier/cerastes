// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/range_get_set.c
// Pins: 0060 items 1 and 3 (M4.8): owns_range and reads_range on one parameter in one clause list are rejected
// Pre-registered from record 0060 items 1 and 3, before the code
#include <cerastes.h>
#include <stddef.h>

void f(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_reads_range(p, n))
{
  (void)p;
  (void)n;
}
