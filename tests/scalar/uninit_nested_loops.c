// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 4: assigned in an inner loop that always runs, inside an outer loop that always runs, read after both
#include <cerastes.h>

int nested(int n)
    CERASTES_REQUIRES(1 <= n && n <= 715827882)
    CERASTES_ENSURES(r, r == 3 * n - 1)
{
  int x;
  int steps = 0;
  int i = 0;
  CERASTES_INVARIANT(i < n && steps == 2 * i)
  CERASTES_DECREASES(n - i)
  do {
    int j = 0;
    CERASTES_INVARIANT(j <= 1 && steps == 2 * i + j)
    CERASTES_DECREASES(1 - j)
    for (;;) {
      x = i;
      steps += 1;
      if (j >= 1) {
        break;
      }
      j += 1;
    }
    i += 1;
  } while (i < n);
  return x + steps;
}
