// expect: rejected cerastes-unsupported_type @ L8
// mode: c23, c++20
// twin: tests/scalar/sizeof_value.c
// Pins: 0024 item 6: sizeof of an array type in an ensures (wrapping bound: the front end says 1)
#include <cerastes.h>

int one(void)
    CERASTES_ENSURES(r, r == (int)sizeof(char[65536u * 65536u + 1]))
{
  return 1;
}
