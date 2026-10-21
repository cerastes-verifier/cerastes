// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0015 item 4: after abort() the path is unreachable, so even check(false) holds there
// The ensures may also use that the negative path never returns.
#include <stdlib.h>
#include <cerastes.h>

int non_negative(int x)
    CERASTES_ENSURES(r, r == x && x >= 0)
{
  if (x < 0) {
    abort();
    CERASTES_CHECK(false);
  }
  return x;
}
