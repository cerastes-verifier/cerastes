// expect: rejected cerastes-unsupported_statement @ L12
// mode: c23, c++20
// twin: tests/scalar/switch_fallthrough.c
// Pins: 0014 item 1: a case label not directly in its switch's braces (Duff's device) is rejected
#include <cerastes.h>

int pick(int x)
{
  switch (x) {
  case 0:
    if (x == 0) {
    case 1:
      return 1;
    }
    return 2;
  default:
    return 3;
  }
}
