// expect: error postcondition @ L14
// mode: c23, c++20
// twin: tests/memory/ptrptr_const_middle.c
// Pins: 0040 item 2: **cp reads x, which is 3
#include <cerastes.h>

int read_const(void)
    CERASTES_ENSURES(r, r == 4)
{
  int x = 3;
  int *p = &x;
  int **pp = &p;
  int *const *cp = pp;
  return **cp;
}
