// expect: error postcondition @ L16
// mode: c23, c++20
// twin: tests/memory/const_pointer.c
// Pins: 0032 item 3: *q sees the write through p, so the sum is 13 and not 12
#include <cerastes.h>

int through_const(void)
    CERASTES_ENSURES(r, r == 12)
{
  int x = 1;
  int *p = &x;
  const int *q = p;
  const int k = 5;
  const int *pk = &k;
  *p = 8;
  return *q + *pk;
}
