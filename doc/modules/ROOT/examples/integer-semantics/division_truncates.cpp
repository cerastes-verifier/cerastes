// expect: verified
// expect: native-ok
// Division and remainder truncate toward zero.
// tag::source[]
#include <cerastes.h>

int quotient(int a)
    CERASTES_REQUIRES(a == -7)
    CERASTES_ENSURES(r, r == -3)
{
  return a / 2;
}

int remainder_of(int a)
    CERASTES_REQUIRES(a == -7)
    CERASTES_ENSURES(r, r == -1)
{
  return a % 2;
}
// end::source[]
