// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 6 with 0015: a goto out of a loop carries the assignment made before the jump
#include <cerastes.h>

int find(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, (n > 5 && r == 5) || (n <= 5 && r == -1))
{
  int x;
  int i = 0;
  CERASTES_INVARIANT(i <= 5)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (i == 5) {
      x = i;
      goto found;
    }
    i += 1;
  }
  x = -1;
found:
  return x;
}
