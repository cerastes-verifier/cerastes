// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 2: a pointer to an inner block's local, stored through a cell, is used inside the block (a loaded pointer is a use: liveness)
#include <cerastes.h>
#include <stddef.h>

int retarget_block(int a)
    CERASTES_REQUIRES(-1000 < a && a < 1000)
    CERASTES_ENSURES(r, r == a + 7)
{
  int x = a;
  int *p = &x;
  int **pp = &p;
  {
    int y = 7;
    *pp = &y;
    if (*pp == &y) {
      x += **pp;
    }
  }
  return x;
}
