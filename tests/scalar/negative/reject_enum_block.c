// expect: rejected cerastes-unsupported_declaration @ L9
// mode: c23, c++20
// twin: tests/scalar/enum_switch.c
// Pins: 0018 item 2: an enumeration declared in a function body is rejected, as every type declaration in a body is (0012 item 12)
#include <cerastes.h>

int f(int x)
{
  enum local { L0, L1 };
  return x;
}
