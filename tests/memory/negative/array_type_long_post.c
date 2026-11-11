// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/memory/array_type_long.c
// Pins: 0059 item 2: the sum is 3000000001
// Pre-registered from record 0059 item 2
#include <cerastes.h>

long wide(void)
    CERASTES_ENSURES(r, r == 3000000000L)
{
  long a[2] = {3000000000L, 1L};
  return a[0] + a[1];
}
