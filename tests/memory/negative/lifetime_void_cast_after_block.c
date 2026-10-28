// expect: error lifetime @ L18
// mode: c23, c++20
// twin: tests/memory/void_cast_pointer.c
// Pins: audit round 2 of M4.5b1a, 0032 item 6: (void)p after the block still uses the value of a dangling pointer
#include <cerastes.h>

int void_casts(int c)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 1;
  int y = 2;
  int *p = &x;
  (void)(c != 0 ? p : &y);
  {
    int z = 3;
    p = &z;
  }
  (void)p;
  return y - 1;
}
