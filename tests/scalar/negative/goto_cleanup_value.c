// expect: error postcondition @ L25
// mode: c23, c++20
// twin: tests/scalar/goto_cleanup.c
// Pins: 0015 item 2: the exact ensures of the positive notices a wrong clamp value
// The upper clamp assigns 99 where the ensures says 100.
#include <cerastes.h>

int clamp_sum(int a, int b)
    CERASTES_REQUIRES(0 <= a && a <= 1000 && 0 <= b && b <= 1000)
    CERASTES_ENSURES(r, r == (a + b > 100 ? 100 : (a + b < 10 ? 9 : a + b)))
{
  int status = 0;
  int total = a + b;
  if (total > 100) {
    status = 99;
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
