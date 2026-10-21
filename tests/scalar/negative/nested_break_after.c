// expect: error check @ L28
// mode: c23, c++20
// twin: tests/scalar/nested_break.c
// Pins: 0014 item 4: after the inner loop only the break path's facts hold
// j == 3 would be the condition exit, which the invariant j <= 1 rules out.
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
    CERASTES_CHECK(j == 3);
    count += 1;
    i += 1;
  }
  return count + t;
}
