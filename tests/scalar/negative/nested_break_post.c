// expect: error postcondition @ L32
// mode: c23, c++20
// twin: tests/scalar/nested_break.c
// Pins: 0014 item 4: the inner break does not leave the outer loop
// The outer loop runs n times: r == 2 * n + 1 never holds.
#include <cerastes.h>

int count_rounds(int n)
    CERASTES_REQUIRES(0 <= n && n <= 1073741823)
    CERASTES_ENSURES(r, r == 2 * n + 1)
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
