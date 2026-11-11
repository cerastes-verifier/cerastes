// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/memory/array_type_uchar.c
// Pins: 0059 item 2: the sum is 255
// Pre-registered from record 0059 item 2
#include <cerastes.h>

int bytes(void)
    CERASTES_ENSURES(r, r == 256)
{
  unsigned char a[2] = {200, 55};
  return a[0] + a[1];
}
