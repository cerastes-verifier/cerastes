// expect: error uninitialized @ L23
// mode: c23, c++20
// twin: tests/scalar/uninit_goto_out_of_loop.c
// Pins: 0019 item 6: the goto leaves the loop without assigning x, and the label reads it
#include <cerastes.h>

int find(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, n > 5 || r == -1)
{
  int x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && i <= 5)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (i == 5) {
      goto found;
    }
    i += 1;
  }
  x = -1;
found:
  return x;
}
