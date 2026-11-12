// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/range_get_set.c
// Pins: 0060 item 3: a range atom with three arguments is rejected: a wrong number of arguments
// Pre-registered from record 0060 items 1 and 3, before the code
#include <cerastes.h>
#include <stddef.h>

void f(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n, 1))
{
  (void)p;
  (void)n;
}
