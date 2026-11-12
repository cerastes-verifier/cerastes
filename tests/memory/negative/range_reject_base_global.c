// expect: rejected cerastes-unsupported_annotation @ L12
// mode: c23, c++20
// twin: tests/memory/range_get_set.c
// Pins: 0060 items 1 and 3 (M4.5e2): a global is not a base
// Pre-registered from record 0060 items 1 and 3, before the code
#include <cerastes.h>
#include <stddef.h>

int *g;

void f(int n)
    CERASTES_REQUIRES(cerastes_owns_range(g, n))
{
  (void)n;
}
