// expect: error check @ L27
// mode: c23, c++20
// twin: tests/scalar/do_once_targets.c
// Pins: 0014 item 2: the code after the do ... while (0) is reached from its break and continue
// a == i fails for i == 3 and i == 4, which skip the increment of a.
#include <cerastes.h>

int skip_count(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, (n <= 3 && r == 0) || (n == 4 && r == 1) || (n >= 5 && r == 2))
{
  int i = 0;
  int a = 0;
  int c = 0;
  CERASTES_INVARIANT(i <= n && c == i && ((i <= 3 && a == i) || (i == 4 && a == 3) || (i >= 5 && a == i - 2)))
  CERASTES_DECREASES(n - i)
  for (i = 0; i < n; i++) {
    do {
      if (i == 3) {
        break;
      }
      if (i == 4) {
        continue;
      }
      a += 1;
    } while (0);
    CERASTES_CHECK(a == i);
    c += 1;
  }
  return c - a;
}
