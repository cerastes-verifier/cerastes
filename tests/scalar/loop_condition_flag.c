// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 13 and 0020: a local written only in a loop condition is initialized after the loop (the condition runs at least once)
#include <cerastes.h>

int drain(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, r == n)
{
  int c;
  int i = 0;
  CERASTES_INVARIANT(i <= n)
  CERASTES_DECREASES(n - i)
  while ((c = n - i) > 0) {
    i++;
  }
  return i - c;
}
