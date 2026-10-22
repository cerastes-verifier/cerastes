// expect: rejected cerastes-unsupported_attribute @ L7
// mode: c23, c++20
// twin: tests/scalar/const_global.c
// Pins: 0017 item 2: a section attribute on a named constant is rejected (another unit or the loader may change the value)
#include <cerastes.h>

__attribute__((section("__DATA,__data"))) const int S = 6;

int get(void)
{
  return S;
}
