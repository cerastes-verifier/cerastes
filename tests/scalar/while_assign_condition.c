// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 3: an assignment in a loop condition is stored at every evaluation, and its arithmetic is checked there
#include <cerastes.h>

int reach(int n)
    CERASTES_REQUIRES(0 <= n && n < 2147483647)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  int c;
  CERASTES_INVARIANT(i <= n)
  CERASTES_DECREASES(n - i)
  while ((c = i + 1) <= n) {
    i = c;
  }
  return i;
}
