// expect: rejected cerastes-unsupported_statement @ L12
// mode: c23, c++20
// twin: tests/scalar/switch_fallthrough.c
// Pins: 0014 item 1: a statement before the first case label is rejected
// The statement is unreachable; the encoder does not guess what it was meant to do.
// (A check, not an assignment: the front end warns about an unreachable assignment, #111, and a test is warning-free.)
#include <cerastes.h>

int pick(int x)
{
  switch (x) {
  CERASTES_CHECK(x == 0);
  case 0:
    return 1;
  default:
    return 0;
  }
}
