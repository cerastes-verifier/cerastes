// expect: rejected cerastes-unsupported_declaration @ L8
// twin: tests/scalar/abs_value.c
// Pins: 0017 B1: in C++ a #pragma weak after a declaration of f makes f weak, so its contract is not enough
#include <cerastes.h>

int f(int x) CERASTES_ENSURES(r, r == x);
#pragma weak f
int f(int x)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}
