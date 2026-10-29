// expect: error overflow @ L16
// mode: c23, c++20
// twin: tests/memory/loop_pointer_write.c
// Pins: 0038 item 3: a write through a pointer havocs the cell to its range, so without a value invariant the increment may overflow
#include <cerastes.h>

int through_pointer(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int x = 0;
  int *p = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p += 1;
    i += 1;
  }
  return x;
}
