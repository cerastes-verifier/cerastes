// expect: rejected cerastes-unsupported_declaration @ L8
// c-only: C++ ignores a #pragma weak that comes before any declaration of the name, as clang does (the .cpp twin declares first); in C the pragma makes it weak
// twin: tests/scalar/const_global.c
// Pins: 0017 item 2: #pragma weak makes the constant weak, so its value is not known here
#include <cerastes.h>

#pragma weak W
extern const int W = 7;

int get(void)
{
  return W;
}
