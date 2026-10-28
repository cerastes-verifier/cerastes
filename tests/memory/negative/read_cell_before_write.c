// expect: error uninitialized @ L12
// mode: c23, c++20
// twin: tests/memory/write_then_read.c
// Pins: 0019 and 0032 item 6: the cell is read by name before any write
#include <cerastes.h>

int through(int a)
    CERASTES_ENSURES(r, r == a)
{
  int x;
  int *p = &x;
  int y = x;
  *p = a;
  return y;
}
