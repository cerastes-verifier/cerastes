// expect: error uninitialized @ L17
// mode: c23, c++20
// twin: tests/scalar/uninit_loop_body_decl.c
// Pins: 0019 item 2: t is a new indeterminate object in each iteration, so the value of the previous iteration is not readable
#include <cerastes.h>

int sum_threes(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int t;
    if (i > 0) {
      s = t;
    }
    t = 3;
    i += 1;
  }
  return s;
}
