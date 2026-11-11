// expect: error unsequenced @ L13
// mode: c23, c++20
// twin: tests/memory/array_unsequenced_distinct.c
// Pins: 0059 item 9 (0039 item 3): without i != j the write and the increment may reach one element
// Pre-registered from record 0059 item 9
#include <cerastes.h>

int shift(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 3 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, 2 <= r && r <= 4)
{
  int a[3] = {1, 2, 3};
  a[i] = a[j]++;
  return a[j];
}
