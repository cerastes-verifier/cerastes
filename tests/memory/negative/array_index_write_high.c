// expect: error out-of-bounds @ L13
// mode: c23, c++20
// twin: tests/memory/array_index_write.c
// Pins: 0059 item 4: the write to a[i] with i == 3 may be outside the array: out-of-bounds at the write
// Pre-registered from record 0059 item 4
#include <cerastes.h>

int put(int i)
    CERASTES_REQUIRES(0 <= i && i <= 3)
    CERASTES_ENSURES(r, r == 7)
{
  int a[3] = {0};
  a[i] = 7;
  return a[i];
}
