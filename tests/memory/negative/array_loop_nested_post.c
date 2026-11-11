// expect: error postcondition @ L36
// mode: c23, c++20
// twin: tests/memory/array_loop_nested.c
// Pins: 0059 item 8: the count is 3, not 2
// Pre-registered from record 0059 item 8 (Part C), before the code
#include <cerastes.h>

int nested(void)
    CERASTES_ENSURES(r, r == 2)
{
  int a[3] = {0};
  int i = 0;
  int c = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_INVARIANT(c == i)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    a[i] = 1;
    int j = 0;
    int t = 0;
    CERASTES_INVARIANT(0 <= j && j <= 3)
    CERASTES_INVARIANT(0 <= t && t <= j)
    CERASTES_DECREASES(3 - j)
    while (j < 3) {
      if (a[j] == 1) {
        t = t + 1;
      }
      j = j + 1;
    }
    if (t > 3) {
      return -1;
    }
    c = c + 1;
    i = i + 1;
  }
  return c;
}
