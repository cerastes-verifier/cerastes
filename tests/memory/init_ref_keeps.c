// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0042 items 5, 7 (question 87): after a ref_to_uninit call an object that was initialized is still initialized (old(flag) ==> flag), so the caller reads it
#include <cerastes.h>

void maybe(int *p CERASTES_REF_TO_UNINIT, int c)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  if (c != 0) {
    *p = 5;
  }
}

int keep(int c)
{
  int x = 1;
  maybe(&x, c);
  return x;
}
