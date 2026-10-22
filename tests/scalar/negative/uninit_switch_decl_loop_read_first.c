// expect: error uninitialized @ L20
// mode: c23, c++20
// twin: tests/scalar/uninit_switch_decl_loop_in_range.c
// Pins: 0019 items 4 and 5: the loop reads x before assigning it, so the read may see the indeterminate value
#include <cerastes.h>

int run(int code, int n)
    CERASTES_REQUIRES(0 <= code && code <= 1 && 0 <= n && n <= 100)
{
  int total = 0;
  switch (code) {
    int x;
  case 0:
    break;
  case 1: {
    int i = 0;
    CERASTES_INVARIANT(0 <= i && i <= n)
    CERASTES_DECREASES(n - i)
    while (i < n) {
      total = x;
      x = i + 1;
      i += 1;
    }
    break;
  }
  }
  return total;
}
