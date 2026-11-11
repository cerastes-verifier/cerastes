// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 8 (0042 item 5): an outer loop that writes a[i] and an inner loop that only reads a[j]: the inner loop keeps the flags and the type's range of every array it holds, reached or not, so the outer loop's rows survive it
// Pre-registered from record 0059 item 8 (Part C), before the code: verifies
#include <cerastes.h>

int nested(void)
    CERASTES_ENSURES(r, r == 3)
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
