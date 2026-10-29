// expect: rejected cerastes-unsupported_expression @ L20
// mode: c23, c++20
// twin: tests/memory/owns_inc.c
// Pins: 0037 items 8 and 15: two calls with permission atoms in one full expression are rejected (re-expected by b1c)
#include <cerastes.h>
#include <stddef.h>

int bump(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == cerastes_old(*p) + 1 && r == *p)
{
  *p += 1;
  return *p;
}

int twice(void)
{
  int x = 1;
  int y = 2;
  return bump(&x) + bump(&y);
}
