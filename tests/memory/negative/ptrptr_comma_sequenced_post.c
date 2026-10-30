// expect: error postcondition @ L16
// mode: c23, c++20
// twin: tests/memory/ptrptr_comma_sequenced.c
// Pins: 0040 item 2: the comma's right operand reads y through the new *pp, which is 5, not 0
#include <cerastes.h>

int sequenced(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 0;
  int y = 5;
  int *p = &x;
  int **pp = &p;
  int r = 0;
  r = (*pp = &y, **pp);
  return r;
}
