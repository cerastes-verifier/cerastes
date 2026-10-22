// expect: rejected cerastes-unsupported_declaration @ L8
// c-only: C++ ignores a #pragma weak that comes before any declaration of the name, as clang does (the .cpp twin declares first); in C the pragma makes it weak
// twin: tests/scalar/abs_value.c
// Pins: 0017 B1: #pragma weak f makes the function weak, so another unit may replace its body and the contract is not enough
#include <cerastes.h>

#pragma weak f
int f(int x)
    CERASTES_REQUIRES(0 <= x && x < 100)
    CERASTES_ENSURES(r, r == x + 1)
{
  return x + 1;
}
