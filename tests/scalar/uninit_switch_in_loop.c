// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 items 4 and 6: a switch body that starts with a declaration, inside a loop; each iteration resets the variable
#include <cerastes.h>

int run(int n, int code)
    CERASTES_REQUIRES(0 <= n && n <= 715827882)
    CERASTES_ENSURES(r, r == (code == 0 ? 2 * n : 3 * n))
{
  int total = 0;
  int i = 0;
  CERASTES_INVARIANT(i <= n && total == (code == 0 ? 2 * i : 3 * i))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    switch (code) {
      int x;
    case 0:
      x = 2;
      total += x;
      break;
    default:
      x = 3;
      total += x;
    }
    i += 1;
  }
  return total;
}
