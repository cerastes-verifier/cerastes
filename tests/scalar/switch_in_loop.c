// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 6: continue in a switch targets the enclosing loop, break the switch
// i == 1 goes back to the test (continue: the loop); i == 2 adds 10 and the others 1, each leaving the switch (break).
#include <cerastes.h>

int tally(int n)
    CERASTES_REQUIRES(0 <= n && n <= 2147483639)
    CERASTES_ENSURES(r, (n <= 1 && r == 0) || (n >= 2 && r == n + 8))
{
  int i = 0;
  int c = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && ((i <= 1 && c == 0) || (i >= 2 && c == i + 8)))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
    switch (i) {
    case 1:
      continue;
    case 2:
      c += 10;
      break;
    default:
      c += 1;
      break;
    }
  }
  return c;
}
