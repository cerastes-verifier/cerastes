// expect: error lifetime @ L17
// mode: c23, c++20
// twin: tests/memory/void_cast_pointer.c
// Pins: audit round 2 of M4.5b1a, 0032 item 6: (void)(c ? p : &y) after the block uses p in an arm
#include <cerastes.h>

int void_casts(int c)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 1;
  int y = 2;
  int *p = &x;
  {
    int z = 3;
    p = &z;
  }
  (void)(c != 0 ? p : &y);
  return y - 1;
}
