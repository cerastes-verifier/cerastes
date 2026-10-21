// expect: rejected cerastes-unsupported_statement @ L13
// mode: c23, c++20
// twin: tests/scalar/goto_out_of_do_once.c
// Pins: 0015 item 2: a goto never enters a do ... while (0) block
// The label is inside the block, after the goto.
#include <cerastes.h>

int enter(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  int y = 0;
  if (x > 5) {
    goto inside;
  }
  do {
    y = 1;
inside:
    y += 1;
  } while (0);
  return y;
}
