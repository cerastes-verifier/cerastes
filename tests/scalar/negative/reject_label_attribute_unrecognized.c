// expect: rejected cerastes-unrecognized_attribute @ L10
// mode: c23, c++20
// twin: tests/scalar/switch_fallthrough.c
// Pins: 0014 item 10: an unrecognized attribute on a case label is rejected
#include <cerastes.h>

int pick(int x)
{
  switch (x) {
  [[foo::bar]] case 1:
    return 1;
  default:
    return 0;
  }
}
