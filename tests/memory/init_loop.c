// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0042 item 5: a loop in a must_init function over the place, the write before the loop: the loop's automatic invariants keep the flag
#include <cerastes.h>

void fill(int *p CERASTES_MUST_INIT, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*p) && *p == n)
{
  int i = 0;
  *p = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && *p == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p += 1;
    i += 1;
  }
}

int use_fill(void)
    CERASTES_ENSURES(r, r == 5)
{
  int x;
  fill(&x, 5);
  return x;
}
