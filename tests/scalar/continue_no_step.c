// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 4: continue in a for loop runs the increment, so the measure decreases
#include <cerastes.h>

int count_not_three(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, (n <= 3 && r == n) || (n > 3 && r == n - 1))
{
  int i = 0;
  int c = 0;
  CERASTES_INVARIANT(i <= n && ((i <= 3 && c == i) || (i > 3 && c == i - 1)))
  CERASTES_DECREASES(n - i)
  for (i = 0; i < n; i++) {
    if (i == 3) {
      continue;
    }
    c += 1;
  }
  return c;
}
