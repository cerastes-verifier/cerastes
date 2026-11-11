// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/memory/array_init_empty.c
// Pins: 0059 item 3: both elements are 0, so r == 1 fails
// Pre-registered from record 0059 item 3
#include <cerastes.h>

int none(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a[2] = {};
  return a[0] + a[1];
}
