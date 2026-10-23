// expect: error overflow @ L15
// mode: c++20
// twin: tests/scalar/unsigned_default_argument.cpp
// Pins: 0022 item 2: the default argument 0u - 1u overflows: the error is at the call, with a note at the default argument
#include <cerastes.h>

unsigned f(unsigned a = 0u - 1u)
    CERASTES_ENSURES(r, r == a)
{
  return a;
}

unsigned g(void)
{
  return f();
}
