// expect: rejected cerastes-unsupported_attribute @ L13
// mode: c23, c++20
// twin: tests/scalar/switch_fallthrough.c
// Pins: 0014 item 10: [[clang::fallthrough]] is not on the allow-list
#include <cerastes.h>

int bonus(int k)
{
  int total = 0;
  switch (k) {
  case 0:
    total += 4;
    [[clang::fallthrough]];
  default:
    total += 2;
    break;
  }
  return total;
}
