// expect: error check @ L14
// mode: c23, c++20
// twin: tests/memory/alias_write.c
// Pins: 0032 items 4, 6: the aliasing trap: x was changed through q, so x == 1 does not hold
#include <cerastes.h>

int alias(void)
    CERASTES_ENSURES(r, r == 3)
{
  int x = 1;
  int *p = &x;
  int *q = p;
  *q = 3;
  CERASTES_CHECK(x == 1);
  return *p;
}
