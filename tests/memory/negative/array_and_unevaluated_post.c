// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/memory/array_and_unevaluated.c
// Pins: 0059 item 4 (as updated): the && is false, so r == 1 fails at the return
// Pre-registered from record 0059 item 4 (as updated)
#include <cerastes.h>

int never(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a[3] = {1, 2, 3};
  return 0 && a[3];
}
