// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 4: assigned in the body of a do loop, read in its condition
#include <cerastes.h>

int count_down(int n)
    CERASTES_REQUIRES(1 <= n)
    CERASTES_ENSURES(r, r == 0)
{
  int x;
  int i = 0;
  CERASTES_INVARIANT(i < n)
  CERASTES_DECREASES(n - i)
  do {
    x = n - i - 1;
    i += 1;
  } while (x > 0);
  return x;
}
