// expect: error postcondition @ L10
// mode: c23, c++20
// twin: tests/scalar/logic_not.c
// Pins: 0010 item 10: !a is 1 exactly when a == 0
#include <cerastes.h>

int is_zero(int a)
    CERASTES_ENSURES(r, r == (a != 0))
{
  return !a;
}
