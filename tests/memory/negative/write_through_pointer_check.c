// expect: error check @ L13
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0032 item 4: the write through p changed x, so x == 1 does not hold
#include <cerastes.h>

int write_through(void)
    CERASTES_ENSURES(r, r == 2)
{
  int x = 1;
  int *p = &x;
  *p = 2;
  CERASTES_CHECK(x == 1);
  return x;
}
