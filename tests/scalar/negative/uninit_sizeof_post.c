// expect: error postcondition @ L11
// mode: c23, c++20
// twin: tests/scalar/uninit_sizeof.c
// Pins: 0019 item 2: sizeof does not read x; the size is not 8
#include <cerastes.h>

int size_of(void)
    CERASTES_ENSURES(r, r == 8)
{
  int x;
  return (int)sizeof(x);
}
