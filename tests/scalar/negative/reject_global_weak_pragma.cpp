// expect: rejected cerastes-unsupported_declaration @ L8
// twin: tests/scalar/const_global.c
// Pins: 0017 item 2: in C++ a #pragma weak after a declaration of W makes it weak, so its value is not known here
#include <cerastes.h>

extern const int W;
#pragma weak W
const int W = 7;

int get()
{
  return W;
}
