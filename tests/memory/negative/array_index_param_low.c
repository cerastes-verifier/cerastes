// expect: error out-of-bounds @ L13
// mode: c23, c++20
// twin: tests/memory/array_index_param.c
// Pins: 0059 item 4: i == -1 may reach the read: out-of-bounds, never a[SIZE_MAX]
// Pre-registered from record 0059 item 4
#include <cerastes.h>

int at(int i)
    CERASTES_REQUIRES(-1 <= i && i < 3)
    CERASTES_ENSURES(r, r == 0)
{
  int a[3] = {0};
  return a[i];
}
