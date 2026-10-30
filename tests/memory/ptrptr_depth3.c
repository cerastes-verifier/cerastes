// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 2 (question 76): pointers to pointers at any depth; pp is itself a cell, so &pp is an int *** whose chain *** writes x
#include <cerastes.h>
#include <stddef.h>

int depth3(void)
    CERASTES_ENSURES(r, r == 9)
{
  int x = 1;
  int *p = &x;
  int **pp = &p;
  int ***ppp = &pp;
  ***ppp = 9;
  return x;
}
