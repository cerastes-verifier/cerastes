// expect: error out-of-bounds @ L13
// mode: c23, c++20
// twin: tests/memory/array_index_param.c
// Pins: 0059 item 4: i == 3 may reach the read of a[i]: out-of-bounds at the access
// Pre-registered from record 0059 item 4
#include <cerastes.h>

int at(int i)
    CERASTES_REQUIRES(0 <= i && i <= 3)
    CERASTES_ENSURES(r, r == 0)
{
  int a[3] = {0};
  return a[i];
}
