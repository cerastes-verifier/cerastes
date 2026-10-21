// expect: rejected cerastes-unsupported_statement @ L13
// mode: c23, c++20
// twin: tests/scalar/goto_out_of_loops.c
// Pins: 0015 item 2: a goto never enters a loop (it would skip the invariant check on entry; Silicon throws)
// The loop has an invariant and a measure, so the goto is the only problem.
#include <cerastes.h>

int enter(int n)
    CERASTES_REQUIRES(0 <= n && n <= 10)
{
  int i = 0;
  if (n > 5) {
    goto inside;
  }
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    i += 1;
inside:
    i += 0;
  }
  return i;
}
