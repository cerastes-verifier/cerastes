// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 items 4 and 5: x is declared at the top of a switch body and assigned in a loop inside a case; the loop's automatic range invariant on x holds on entry only because the reset before the dispatch gave x an in-range value
#include <cerastes.h>

int run(int code, int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, r == (code == 1 ? n : 0))
{
  int total = 0;
  switch (code) {
    int x;
  case 0:
    break;
  case 1: {
    int i = 0;
    CERASTES_INVARIANT(i <= n && total == i)
    CERASTES_DECREASES(n - i)
    while (i < n) {
      x = i + 1;
      total = x;
      i += 1;
    }
  }
  }
  return total;
}
