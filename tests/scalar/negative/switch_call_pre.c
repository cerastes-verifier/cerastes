// expect: error precondition @ L19
// mode: c23, c++20
// twin: tests/scalar/switch_call.c
// Pins: 0014 item 6: the call in the scrutinee is checked against its precondition, at the switch
// dispatch now admits x == INT_MAX, past succ's limit.
#include <cerastes.h>

int succ(int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1)
{
  return x + 1;
}

int dispatch(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, (x == 0 && r == 100) || (x == 1 && r == 200) || (x >= 2 && r == 0))
{
  switch (succ(x)) {
  case 1:
    return 100;
  case 2:
    return 200;
  default:
    return 0;
  }
}
