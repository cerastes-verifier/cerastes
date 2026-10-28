// expect: error uninitialized @ L16
// mode: c23, c++20
// twin: tests/memory/pointer_uninit.c
// Pins: 0019 and 0032 item 2: the pointer is never assigned when c == 0
#include <cerastes.h>

int select_cell(int c)
    CERASTES_ENSURES(r, r == (c != 0 ? 1 : 2))
{
  int x = 1;
  int y = 2;
  int *p;
  if (c != 0) {
    p = &x;
  }
  return *p + y - 2;
}
