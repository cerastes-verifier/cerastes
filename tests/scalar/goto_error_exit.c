// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0015 item 2: a forward goto to a label of an enclosing block (the error-exit idiom)
// Each goto sets the error code and jumps to the label; after the label the facts of both paths hold.
#include <cerastes.h>

int checked_sum(int x, int y)
    CERASTES_REQUIRES(0 <= x && x <= 100 && 0 <= y && y <= 100)
    CERASTES_ENSURES(r, (r == -1 && x > 50) || (r == -2 && x <= 50 && y > 50) || (r == x + y && x <= 50 && y <= 50))
{
  int err = 0;
  if (x > 50) {
    err = -1;
    goto fail;
  }
  if (y > 50) {
    err = -2;
    goto fail;
  }
  return x + y;
fail:
  return err;
}
