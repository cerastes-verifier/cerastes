// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 2 (P62 A5): int ** to int *const * adds a qualifier to the pointee only: accepted in both modes, the same cell
#include <cerastes.h>

int read_const(void)
    CERASTES_ENSURES(r, r == 3)
{
  int x = 3;
  int *p = &x;
  int **pp = &p;
  int *const *cp = pp;
  return **cp;
}
