// expect: error precondition @ L16
// A program that fails with the precondition error.
// tag::source[]
#include <cerastes.h>

int half(int n)
    CERASTES_REQUIRES(n >= 0 && n % 2 == 0)
    CERASTES_ENSURES(r, r == n / 2)
{
  return n / 2;
}

int use_half(int m)
    CERASTES_REQUIRES(m >= 0)
{
  return half(m);
}
// end::source[]
