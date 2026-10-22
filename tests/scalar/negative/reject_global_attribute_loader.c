// expect: rejected cerastes-unrecognized_attribute @ L7
// c-only: C++ requires an initializer for a const object (error #257)
// twin: tests/scalar/const_global_tentative.c
// Pins: 0017 item 2: an attribute on a named constant is held to the allow-list (loader_uninitialized changes its value)
#include <cerastes.h>

static const int z __attribute__((loader_uninitialized));

int f(void)
    CERASTES_ENSURES(r, r == 0)
{
  return z;
}
