// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/memory/array_sizeof_unevaluated.c
// Pins: 0059 item 4 (as updated): sizeof(int) is 4, a[0] is 1: r == 4 fails at the return
// Pre-registered from record 0059 item 4 (as updated)
#include <cerastes.h>

int size(void)
    CERASTES_ENSURES(r, r == 4)
{
  int a[3] = {1, 2, 3};
  return (int)sizeof(a[3]) + a[0];
}
