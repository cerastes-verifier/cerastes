// expect: error postcondition @ L16
// mode: c23, c++20
// twin: tests/memory/range_guards.c
// Pins: 0060 item 9: under the guard the element is p[0], not 0: the postcondition fails at that return only (the guard-false return is fine)
// Pre-registered from record 0060 item 9, before the code
#include <cerastes.h>
#include <stddef.h>

int first_or(int *p, int n)
    CERASTES_REQUIRES(n == 0 || cerastes_owns_range(p, n))
    CERASTES_ENSURES(r, (n == 0 || cerastes_owns_range(p, n)) && r == 0)
{
  if (n == 0) {
    return 0;
  }
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
