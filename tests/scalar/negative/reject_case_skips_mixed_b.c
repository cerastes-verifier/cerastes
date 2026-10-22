// expect: rejected cerastes-unsupported_statement @ L14
// mode: c23, c++20
// twin: tests/scalar/switch_default_middle.c
// Pins: 0014 item 1 / 0019 item 6: the same, with the initialized declarator first (int y = 1, x;)
#include <cerastes.h>

int skip(int a)
{
  switch (a) {
  case 0:;
    int y = 1, x;
    x = a;
    a = y + x;
  case 1:
    return a;
  default:
    return 0;
  }
}
