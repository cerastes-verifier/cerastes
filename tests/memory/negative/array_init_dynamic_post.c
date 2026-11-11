// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/memory/array_init_dynamic.c
// Pins: 0059 item 3: the second element is y + 1, so the sum is x + y + 1
// Pre-registered from record 0059 item 3
#include <cerastes.h>

int dynamic(int x, int y)
    CERASTES_REQUIRES(0 <= x && x < 100 && 0 <= y && y < 100)
    CERASTES_ENSURES(r, r == x + y)
{
  int a[3] = {x, y + 1, 7};
  return a[0] + a[1];
}
