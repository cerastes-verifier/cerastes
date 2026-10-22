// expect: error postcondition @ L22
// mode: c23, c++20
// twin: tests/scalar/uninit_for_ever_break.c
// Pins: 0019 item 4: the value after the loop is known, and it is not n + 10
#include <cerastes.h>

int find_ten(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == n + 10)
{
  int x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  for (;;) {
    x = i + 11;
    if (i >= n) {
      break;
    }
    i += 1;
  }
  return x;
}
