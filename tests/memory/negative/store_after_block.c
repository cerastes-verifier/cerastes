// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/pointer_scope_inner.c
// Pins: audit round 2 of M4.5b1a, 0032 item 6: a write through a pointer to a local of a block that has ended is a permission error at the store
#include <cerastes.h>

int store_after_block(int a)
    CERASTES_ENSURES(r, r == a)
{
  int x = a;
  int *p = &x;
  {
    int y = 1;
    p = &y;
  }
  *p = 1;
  return a;
}
