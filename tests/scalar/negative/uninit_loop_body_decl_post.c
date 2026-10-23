// expect: error invariant-preserved @ L13
// mode: c23, c++20
// twin: tests/scalar/uninit_loop_body_decl.c
// Pins: 0019 item 4: the body-local is assigned and read, with the wrong value
#include <cerastes.h>

int sum_threes(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 3 * n)
{
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s == 3 * i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int t;
    t = 4;
    s += t;
    i += 1;
  }
  return s;
}
