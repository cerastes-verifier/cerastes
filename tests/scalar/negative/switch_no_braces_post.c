// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/scalar/switch_no_braces.c
// Pins: 0014 item 6: the value that matches no case continues after the switch
#include <cerastes.h>

int one(int x)
    CERASTES_ENSURES(r, (x == 1 && r == 1) || (x != 1 && r == 1))
{
  switch (x)
  case 1:
    return 1;
  return 0;
}
