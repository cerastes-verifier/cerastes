// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 3: a do loop's condition may have an effect; the invariant holds at the start of every iteration
#include <cerastes.h>

int count_do(int n)
    CERASTES_REQUIRES(0 <= n && n <= 715827881)
    CERASTES_ENSURES(r, r == 3 * n + 3)
{
  int i = 0;
  int total = 0;
  CERASTES_INVARIANT(i <= n && total == 3 * i)
  CERASTES_DECREASES(n - i)
  do {
    total += 3;
  } while (i++ < n);
  return total;
}
