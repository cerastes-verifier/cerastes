// expect: rejected cerastes-unsupported_declaration @ L7
// mode: c23, c++20
// twin: tests/scalar/const_global.c
// Pins: 0017 item 2: an extern const that this unit does not define has no known value
#include <cerastes.h>

extern const int E;

int get(void)
{
  return E;
}
