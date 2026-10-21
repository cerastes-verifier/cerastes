// expect: error check @ L24
// mode: c23, c++20
// twin: tests/scalar/goto_cleanup.c
// Pins: 0015 item 2: a check after the label holds only if it holds on every incoming path
// status is 9 on the second goto: status >= 10 fails there and on no other path.
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
  CERASTES_CHECK(10 <= status && status <= 100);
  return status;
}
