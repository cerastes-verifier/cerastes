// expect: error check @ L13
// mode: c23, c++20
// twin: tests/memory/check_through_pointer.c
// Pins: 0032 item 4 (annotations): *p holds 1 after the write, so *p == 2 does not hold
#include <cerastes.h>

int check_deref(void)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 0;
  int *p = &x;
  *p = 1;
  CERASTES_CHECK(*p == 2);
  CERASTES_CHECK(p == &x);
  return x;
}
