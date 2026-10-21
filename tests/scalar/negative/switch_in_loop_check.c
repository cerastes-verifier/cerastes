// expect: error check @ L22
// mode: c23, c++20
// twin: tests/scalar/switch_in_loop.c
// Pins: 0014 item 6: the state inside a case of a switch in a loop
// At i == 2 the count is still 0 (only i == 1 came before, and it adds nothing).
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
      CERASTES_CHECK(c == 1);
      c += 10;
      break;
    default:
      c += 1;
      break;
    }
  }
  return c;
}
