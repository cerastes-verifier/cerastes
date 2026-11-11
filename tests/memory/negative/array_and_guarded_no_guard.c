// expect: error out-of-bounds @ L13
// mode: c23, c++20
// twin: tests/memory/array_and_guarded.c
// Pins: 0059 item 4 (as updated): without the guard a[i] is reached with i == 3
// Pre-registered from record 0059 item 4 (as updated)
#include <cerastes.h>

int guarded(int i)
    CERASTES_REQUIRES(0 <= i && i <= 3)
    CERASTES_ENSURES(r, r == (i < 3))
{
  int a[3] = {0};
  return a[i] == 0;
}
