// expect: error uninitialized @ L13
// mode: c23, c++20
// twin: tests/memory/array_no_init.c
// Pins: 0059 item 3 (0019, path-precise): a[2] was never written
// Pre-registered from record 0059 item 3
#include <cerastes.h>

int one(void)
    CERASTES_ENSURES(r, r == 7)
{
  int a[3];
  a[1] = 7;
  return a[2];
}
