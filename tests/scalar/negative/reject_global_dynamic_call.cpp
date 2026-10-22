// expect: rejected cerastes-unsupported_declaration @ L13
// twin: tests/scalar/const_global.c
// Pins: 0017 item 2: a const global initialized by a call is dynamically initialized
#include <cerastes.h>

int twice(int x)
    CERASTES_REQUIRES(0 <= x && x < 1000)
    CERASTES_ENSURES(r, r == 2 * x)
{
  return 2 * x;
}

const int D = twice(2);

int get()
{
  return D;
}
