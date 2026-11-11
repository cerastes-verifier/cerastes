// expect: error out-of-bounds @ L13
// mode: c23, c++20
// twin: tests/memory/array_index_signed_char.c
// Pins: 0059 item 4: c == -1 may reach the read
// Pre-registered from record 0059 item 4
#include <cerastes.h>

int pick(signed char c)
    CERASTES_REQUIRES(-1 <= c && c < 3)
    CERASTES_ENSURES(r, r == 0)
{
  int a[3] = {0};
  return a[c];
}
