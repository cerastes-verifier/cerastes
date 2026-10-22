// expect: rejected cerastes-unsupported_expression @ L6
// twin: tests/scalar/const_global.c
// Pins: 0017 item 2: an initializer outside the allow-list (a temporary int()) is rejected like any such expression
#include <cerastes.h>

const int z = int();

int get()
{
  return z;
}
