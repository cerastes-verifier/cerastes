// expect: rejected cerastes-unsupported_expression @ L19
// mode: c23, c++20
// twin: tests/memory/owns_inc.c
// Pins: 0037 items 8 and 15: the read of a compound assignment's target is an access (x += f(&x); re-expected by M4.5b1c2)
#include <cerastes.h>
#include <stddef.h>

int bump(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == cerastes_old(*p) + 1 && r == *p)
{
  *p += 1;
  return *p;
}

int compound(void)
{
  int x = 1;
  x += bump(&x);
  return x;
}
