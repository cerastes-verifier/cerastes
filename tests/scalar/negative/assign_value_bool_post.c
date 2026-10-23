// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/scalar/assign_value_bool.c
// Pins: 0020 item 1: the value of (b = 0) is false
#include <cerastes.h>

int flag(int a)
    CERASTES_ENSURES(r, r == (a != 0 ? 2 : 1))
{
  bool b;
  int t = (b = a) ? 1 : 0;
  return t + (b ? 1 : 0);
}
