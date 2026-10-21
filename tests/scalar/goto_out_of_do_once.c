// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0015 item 2: a goto may leave a do ... while (0) block (0014 item 5: straight-line code)
// The statements after the jump in the block are skipped; after the label both paths are known.
#include <cerastes.h>

int guarded(int x)
    CERASTES_REQUIRES(0 <= x && x <= 100)
    CERASTES_ENSURES(r, (x > 50 && r == -1) || (x <= 50 && r == x * 2))
{
  int result = -1;
  do {
    if (x > 50) {
      goto out;
    }
    result = x * 2;
  } while (0);
out:
  return result;
}
