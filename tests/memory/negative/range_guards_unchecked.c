// expect: error null-dereference @ L14
// expect: error lifetime @ L14
// mode: c23, c++20
// twin: tests/memory/range_guards.c
// Pins: 0060 item 4 (S44 (a)): p[0] without the n == 0 test: where the guard is false the atom grants nothing and nothing is known of p, so the access's first rows fail and end the path: null-dereference and lifetime at the read (the guard-true path is non-vacuous: audit round 1, N2)
// Pre-registered from record 0060 items 4 and 9, before the code
#include <cerastes.h>
#include <stddef.h>

int first_or(int *p, int n)
    CERASTES_REQUIRES(n == 0 || cerastes_owns_range(p, n))
    CERASTES_ENSURES(r, n == 0 || cerastes_owns_range(p, n))
{
  return p[0];
}

int first_or_null(int *p, int n)
    CERASTES_REQUIRES(p == NULL || cerastes_owns_range(p, n))
    CERASTES_ENSURES(r, p == NULL || cerastes_owns_range(p, n))
{
  if (p == NULL || n <= 0) {
    return 0;
  }
  return p[0];
}
