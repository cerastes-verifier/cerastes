// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0013 item 2: a for loop's measure is compared from the start of an iteration to the end of the increment
#include <cerastes.h>

int count(int n)
    CERASTES_REQUIRES(n >= 0 && n <= 100)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  int c = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && c == i)
  CERASTES_DECREASES(n - i)
  for (i = 0; i < n; i++) {
    c += 1;
  }
  return c;
}
