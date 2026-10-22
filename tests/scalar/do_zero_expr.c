// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 5: a condition that folds to 0 exempts the do loop; continue goes to the (false) condition
// The condition 1 - 1 is folded to 0 by the front end; continue jumps to it and leaves the loop.
#include <cerastes.h>

int ten_over(int x)
    CERASTES_ENSURES(r, (x == 0 && r == 0) || (x != 0 && r == 10 / x))
{
  int y = 0;
  do {
    if (x == 0) {
      continue;
    }
    y = 10 / x;
  } while (1 - 1);
  return y;
}
