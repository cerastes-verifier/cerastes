// expect: error check @ L16
// mode: c23, c++20
// twin: tests/memory/ptrptr_retarget.c
// Pins: 0040 item 2: after *pp = &y the old value p == &x no longer holds
#include <cerastes.h>
#include <stddef.h>

int retarget(void)
    CERASTES_ENSURES(r, r == 7)
{
  int x = 1;
  int y = 7;
  int *p = &x;
  int **pp = &p;
  *pp = &y;
  CERASTES_CHECK(p == &x);
  return *p;
}
