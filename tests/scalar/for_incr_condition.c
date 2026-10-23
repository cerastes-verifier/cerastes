// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 3: a for loop's condition may have an effect; the step runs after the body and before the next evaluation
#include <cerastes.h>

int count_for(int n)
    CERASTES_REQUIRES(0 <= n && n <= 715827882)
    CERASTES_ENSURES(r, r == 3 * n + 1)
{
  int i;
  int total = 0;
  CERASTES_INVARIANT(i <= n && total == 2 * i)
  CERASTES_DECREASES(n - i)
  for (i = 0; i++ < n; total += 2) {
  }
  return total + i;
}
