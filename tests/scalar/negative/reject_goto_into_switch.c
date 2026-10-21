// expect: rejected cerastes-unsupported_statement @ L13
// mode: c23, c++20
// twin: tests/scalar/goto_out_of_switch.c
// Pins: 0015 item 2: a goto never enters a switch body (the dispatch and its case-value checks would be skipped)
// The label is inside a case of a later switch.
#include <cerastes.h>

int enter(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  int y = 0;
  if (x > 5) {
    goto inside;
  }
  switch (x) {
  case 1:
    y = 1;
inside:
    y += 1;
    break;
  default:
    break;
  }
  return y;
}
