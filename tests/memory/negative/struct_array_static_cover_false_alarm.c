// expect: error permission @ L18
// mode: c23, c++20
// twin: tests/memory/static_local_result.c
// Pins: 0063 item 7 and audit round 3 N4 (a pinned false alarm, fail closed): ensures(r, owns(*r)) of a function that returns the address of an element of a static structure local's array member: the implicit atom's cover skips array leaves (cov_of, leaf_cover_read), so the return reports that it gives *r without holding it; the cell case (static_local_result.c) verifies
// Pre-registered from audit round 3 N4 and record 0063 item 7 (Part C), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int *get(void)
    CERASTES_ENSURES(r, cerastes_owns(*r))
{
  static struct S c;
  return &c.a[1];  // <- here
}
