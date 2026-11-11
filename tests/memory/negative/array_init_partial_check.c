// expect: error check @ L13
// mode: c23, c++20
// twin: tests/memory/array_init_partial.c
// Pins: 0059 item 3: a[3] is 0, not 1
// Pre-registered from record 0059 item 3
#include <cerastes.h>

int partial(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a[4] = {1};
  CERASTES_CHECK(a[0] == 1);
  CERASTES_CHECK(a[3] == 1);
  return a[0] + a[3];
}
