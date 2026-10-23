// expect: rejected cerastes-unsupported_statement @ L14
// mode: c23, c++20
// twin: tests/scalar/switch_default_middle.c
// Pins: 0014 item 1 / 0019 item 6: a case label never skips a declaration with an initializer (int x, y = 1;)
#include <cerastes.h>

int skip(int a)
{
  switch (a) {
  case 0:;
    int x, y = 1;
    x = a;
    a = y + x;
  case 1:
    return a;
  default:
    return 0;
  }
}
