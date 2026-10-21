// expect: rejected cerastes-unsupported_statement @ L8
// twin: tests/scalar/switch_fallthrough.c
// Pins: 0014 item 1: a switch with an init-statement (C++17) is rejected
#include <cerastes.h>

int pick(int x)
{
  switch (int y = x + 1; y) {
  case 1:
    return 1;
  default:
    return 0;
  }
}
