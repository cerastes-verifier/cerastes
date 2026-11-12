// expect: rejected cerastes-unsupported_annotation @ L14
// mode: c23, c++20
// twin: tests/memory/range_get_set.c
// Pins: 0060 item 3 (e1e): a range over structures is rejected
// Pre-registered from record 0060 items 1 and 3, before the code
#include <cerastes.h>
#include <stddef.h>

struct S {
  int x;
};

void f(struct S *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
{
  (void)p;
  (void)n;
}
