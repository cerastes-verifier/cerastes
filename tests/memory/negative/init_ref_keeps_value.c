// expect: error check @ L20
// mode: c23, c++20
// twin: tests/memory/init_ref_keeps.c
// Pins: 0042 items 5, 7 (question 87, audit S4): only the flag is kept after a ref_to_uninit call, not the value: the check x == 1 fails
#include <cerastes.h>

void maybe(int *p CERASTES_REF_TO_UNINIT, int c)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  if (c != 0) {
    *p = 5;
  }
}

void keep(int c)
{
  int x = 1;
  maybe(&x, c);
  CERASTES_CHECK(x == 1);  // <- here
}
