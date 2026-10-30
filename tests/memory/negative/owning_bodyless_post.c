// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/memory/owning_bodyless.c
// Pins: 0040 item 3: put gives the cell back but says nothing of its value, so x == 0 is not known after the call
#include <cerastes.h>
#include <stddef.h>

void put(int *p CERASTES_OWNING);

int use_put(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 0;
  put(&x);
  return x;
}
