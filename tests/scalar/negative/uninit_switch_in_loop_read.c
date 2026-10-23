// expect: error uninitialized @ L22
// mode: c23, c++20
// twin: tests/scalar/uninit_switch_in_loop.c
// Pins: 0019 items 4 and 6: the default reads the declaration at the top of the switch body without assigning it
#include <cerastes.h>

int run(int n, int code)
    CERASTES_REQUIRES(0 <= n && n <= 100 && 0 <= code && code <= 1)
{
  int total = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    switch (code) {
      int x;
    case 0:
      x = 2;
      total = x;
      break;
    default:
      total = x;
      break;
    }
    i += 1;
  }
  return total;
}
