// expect: error postcondition @ L10
// mode: c23, c++20
// twin: tests/scalar/comma_void_cast.c
// Pins: 0020 item 2: the comma's value is its right operand
#include <cerastes.h>

int last(int a, int b)
    CERASTES_ENSURES(r, r == a)
{
  return ((void)a, b);
}
