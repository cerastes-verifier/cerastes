// expect: error unsequenced @ L14
// mode: c23, c++20
// twin: tests/memory/array_unsequenced_pointer.c
// Pins: 0059 item 9 (0039 item 3): with i == 1 the store through p and the increment reach one element
// Pre-registered from record 0059 item 9
#include <cerastes.h>

int through(int i)
    CERASTES_REQUIRES(0 <= i && i < 3)
    CERASTES_ENSURES(r, 2 <= r && r <= 4)
{
  int a[3] = {1, 2, 3};
  int *p = &a[1];
  *p = a[i]++;
  return a[i];
}
