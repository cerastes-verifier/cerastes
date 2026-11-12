// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 11 (amended): a pointer q into a local array (q = a), read in a loop invariant as q[0] and as q[i] with 0 <= i <= 3: the local array's place frames the reads (the interval of its four slots), and the read-only loop keeps the values, so the invariant is inductive and q[0] == 1 holds after the loop
// Pre-registered from record 0060 item 11 (Part D), before the code: verifies
#include <cerastes.h>

int walk(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a[4] = {1, 2, 3, 4};
  int *q = a;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && q[0] == 1 && q[i] >= 1) CERASTES_DECREASES(3 - i) while (i < 3) {
    i++;
  }
  return q[0];
}
