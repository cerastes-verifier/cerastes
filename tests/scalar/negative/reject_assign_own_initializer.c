// expect: rejected cerastes-unsupported_declaration @ L9
// mode: c23, c++20
// twin: tests/scalar/assign_value.c
// Pins: 0012 item 3: int y = (y = 1) names y in its own initializer (rejected in both modes)
#include <cerastes.h>

int f(void)
{
  int y = (y = 1);
  return y;
}
