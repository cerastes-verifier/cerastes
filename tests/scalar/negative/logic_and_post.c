// expect: error postcondition @ L10
// mode: c23, c++20
// twin: tests/scalar/logic_and.c
// Pins: 0010 item 10: (a > 0 || a < 10) is always true, the result is not
#include <cerastes.h>

int in_range(int a)
    CERASTES_ENSURES(r, r == (a > 0 || a < 10))
{
  return a > 0 && a < 10;
}
