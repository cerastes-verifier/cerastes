// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0046 item 6 (question 116): a contract identical to vlibc's on a redeclaration after `<stdlib.h>` is accepted (the front end discards it); vlibc's own is the one that is read
// The repeated `ensures(false)` changes nothing: the path of x < 0 still ends in exit.
#include <stdlib.h>
#include <cerastes.h>

void exit(int status) CERASTES_ENSURES(false);

int nonneg(int x)
    CERASTES_REQUIRES(-10 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x && r >= 0)
{
  if (x < 0) {
    exit(1);
  }
  return x;
}
