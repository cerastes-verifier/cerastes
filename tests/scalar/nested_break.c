// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 4: a break leaves its own loop only
// The inner loop can only be left by its break (j <= 1 < 3); the outer loop goes on.
#include <cerastes.h>

int count_rounds(int n)
    CERASTES_REQUIRES(0 <= n && n <= 1073741823)
    CERASTES_ENSURES(r, r == 2 * n)
{
  int i = 0;
  int count = 0;
  int t = 0;
  CERASTES_INVARIANT(i <= n && count == i && t == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int j = 0;
    CERASTES_INVARIANT(j <= 1 && t == i + j)
    CERASTES_DECREASES(3 - j)
    while (j < 3) {
      if (j == 1) {
        break;
      }
      j += 1;
      t += 1;
    }
    CERASTES_CHECK(j == 1);
    count += 1;
    i += 1;
  }
  return count + t;
}
