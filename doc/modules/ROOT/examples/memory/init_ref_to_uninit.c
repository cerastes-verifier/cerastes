// expect: verified
// expect: native-ok
// mode: c23, c++20
// ref_to_uninit: the callee may receive an object without a value but promises none; an object that had a value keeps it.
// tag::source[]
#include <cerastes.h>

int fill(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 7)
{
  *p = 7;
  return *p;
}

void maybe(int *p CERASTES_REF_TO_UNINIT, int c)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  if (c != 0) {
    *p = 5;
  }
}

int use_both(int c)
    CERASTES_ENSURES(r, r == 7)
{
  int x;
  int y = fill(&x);
  int z = 1;
  maybe(&z, c);
  return y;
}

int keep(int c)
{
  int z = 1;
  maybe(&z, c);
  return z;  // z still has a value: 1 or 5
}
// end::source[]
