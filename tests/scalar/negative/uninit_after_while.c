// expect: error uninitialized @ L18
// mode: c23, c++20
// twin: tests/scalar/uninit_before_loop.c
// Pins: 0019 item 4: assigned only inside a loop that may run zero times, read after it
#include <cerastes.h>

int last_step(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int step;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    step = 2;
    i += 1;
  }
  return step;
}
