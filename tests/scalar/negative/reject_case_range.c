// expect: rejected cerastes-unsupported_statement @ L13
// mode: c23, c++20
// twin: tests/scalar/switch_fallthrough.c
// Pins: 0014 item 1: a GNU case range is rejected (EDG accepts it in both modes)
// Placed after an ordinary case: a range alone first in the body is also reported as a statement before the first label.
#include <cerastes.h>

int pick(int x)
{
  switch (x) {
  case 0:
    return 5;
  case 1 ... 3:
    return 1;
  default:
    return 0;
  }
}
