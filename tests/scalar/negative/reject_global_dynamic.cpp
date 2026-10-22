// expect: rejected cerastes-unsupported_declaration @ L6
// twin: tests/scalar/const_global.c
// Pins: 0017 items 2 and 8: in C++ the initializer is not a constant expression, so D is dynamically initialized
#include <cerastes.h>

const int D = 2147483647 + 1;

int get()
{
  return D;
}
