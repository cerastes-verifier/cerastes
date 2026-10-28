// expect: error permission @ L17
// mode: c23, c++20
// twin: tests/memory/pointer_scope_inner.c
// Pins: 0032 items 6, 8: *p after the block reads y, whose block has ended: the permission is gone
#include <cerastes.h>

int inner_scope(int a)
    CERASTES_REQUIRES(-1000 < a && a < 1000)
    CERASTES_ENSURES(r, r == a + 1)
{
  int x = a;
  int *p = &x;
  {
    int y = 1;
    p = &y;
  }
  x += *p;
  return x;
}
