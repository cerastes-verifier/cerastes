// expect: error postcondition @ L20
// mode: c23, c++20
// twin: tests/memory/array_block_scopes.c
// Pins: 0059 item 3: the second block's array is zero, so s stays 3
// Pre-registered from record 0059 item 3
#include <cerastes.h>

int two_blocks(void)
    CERASTES_ENSURES(r, r == 4)
{
  int s = 0;
  {
    int a[2] = {1, 2};
    s = a[0] + a[1];
  }
  {
    int b[2] = {0};
    s = s + b[0];
  }
  return s;
}
