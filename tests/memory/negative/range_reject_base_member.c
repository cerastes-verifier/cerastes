// expect: rejected cerastes-unsupported_annotation @ L14
// mode: c23, c++20
// twin: tests/memory/range_get_set.c
// Pins: 0060 items 1 and 3 (M4.5e2, members: e1e): a member is not a base
// Pre-registered from record 0060 items 1 and 3, before the code
#include <cerastes.h>
#include <stddef.h>

struct S {
  int *q;
};

void f(struct S *s, int n)
    CERASTES_REQUIRES(cerastes_owns(*s) && cerastes_owns_range(s->q, n))
{
  (void)s;
  (void)n;
}
