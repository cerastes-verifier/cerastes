// expect: error out-of-bounds @ L17
// mode: c23, c++20
// twin: tests/memory/array_loop_fill.c
// Pins: 0059 item 8: with n up to 4 the write of the last iteration is a[3]: out-of-bounds at the write
// Pre-registered from record 0059 item 8 (Part C), before the code
#include <cerastes.h>

int fill(int n)
    CERASTES_REQUIRES(0 <= n && n <= 4)
    CERASTES_ENSURES(r, r == n)
{
  int a[3] = {0};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    a[i] = i;
    i = i + 1;
  }
  return i + a[0] * 0;
}
