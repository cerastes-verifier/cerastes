// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 4: for (;;) and while (1) have no exit but their breaks
#include <cerastes.h>

int count_up(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(i <= n)
  CERASTES_DECREASES(n - i)
  for (;;) {
    if (i >= n) {
      break;
    }
    i += 1;
  }
  return i;
}

int count_down(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, r == 0)
{
  int i = n;
  CERASTES_INVARIANT(0 <= i)
  CERASTES_DECREASES(i)
  while (1) {
    if (i <= 0) {
      break;
    }
    i -= 1;
  }
  return i;
}
