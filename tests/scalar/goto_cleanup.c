// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0015 item 2: several gotos to one label, and a fall-through into it
// After the label the facts of the three incoming paths are known: status is 100, 9 or total.
#include <cerastes.h>

int clamp_sum(int a, int b)
    CERASTES_REQUIRES(0 <= a && a <= 1000 && 0 <= b && b <= 1000)
    CERASTES_ENSURES(r, 9 <= r && r <= 100)
{
  int status = 0;
  int total = a + b;
  if (total > 100) {
    status = 100;
    goto done;
  }
  if (total < 10) {
    status = 9;
    goto done;
  }
  status = total;
done:
  CERASTES_CHECK(9 <= status && status <= 100);
  return status;
}
