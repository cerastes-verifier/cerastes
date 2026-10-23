// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 2: a comma in a loop condition runs its effects at every evaluation of the condition
#include <cerastes.h>

int count(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  int c;
  CERASTES_INVARIANT(i <= n)
  CERASTES_DECREASES(n - i)
  while (c = i, c < n) {
    i++;
  }
  return i;
}
