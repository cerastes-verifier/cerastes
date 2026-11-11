// expect: error unsequenced @ L13
// mode: c23, c++20
// twin: tests/memory/array_unsequenced_offset.c
// Pins: 0059 item 9 (0039 item 3): without i + 1 != j the write to a[i + 1] and the increment of a[j] may reach one element
// Pre-registered from record 0059 item 9
#include <cerastes.h>

int offset(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, 2 <= r && r <= 4)
{
  int a[3] = {1, 2, 3};
  a[i + 1] = a[j]++;
  return a[j];
}
