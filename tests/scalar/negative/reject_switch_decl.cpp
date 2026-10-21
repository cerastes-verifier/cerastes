// expect: rejected cerastes-unsupported_statement @ L8
// twin: tests/scalar/switch_fallthrough.c
// Pins: 0014 item 1: a switch whose condition declares a variable is rejected
#include <cerastes.h>

int pick(int x)
{
  switch (int y = x + 1) {
  case 1:
    return 1;
  default:
    return 0;
  }
}
