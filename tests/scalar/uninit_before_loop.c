// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 4: assigned before a loop, read in it; the loop's flag invariant keeps the fact
#include <cerastes.h>

int count_by_two(int n)
    CERASTES_REQUIRES(0 <= n && n <= 1073741823)
    CERASTES_ENSURES(r, r == 2 * n)
{
  int step;
  int i = 0;
  int s = 0;
  step = 2;
  CERASTES_INVARIANT(i <= n && s == 2 * i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s += step;
    i += 1;
  }
  return s;
}
