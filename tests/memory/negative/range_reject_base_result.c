// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/range_get_set.c
// Pins: 0060 items 1 and 3 (M4.5e2): the base of a range atom is a pointer parameter, never the result
// Pre-registered from record 0060 items 1 and 3, before the code
#include <cerastes.h>
#include <stddef.h>

int *f(int n)
    CERASTES_ENSURES(r, cerastes_owns_range(r, n))
{
  (void)n;
  return NULL;
}
