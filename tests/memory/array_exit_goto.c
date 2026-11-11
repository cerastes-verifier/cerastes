// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 6 (0032 item 8): a forward goto out of the array's block runs the scope exit at the jump, and the block's own end for the path that falls through
// Pre-registered from the M4.5e1a sweep (report 5, s11): verifies
#include <cerastes.h>

int leave(int c)
    CERASTES_ENSURES(r, r == 0)
{
  {
    int a[2] = {1, 2};
    if (c != 0) {
      goto out;
    }
    a[0] = a[1] + 1;
  }
out:
  return 0;
}
