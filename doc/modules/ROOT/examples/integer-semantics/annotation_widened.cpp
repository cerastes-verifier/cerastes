// expect: verified
// expect: native-ok
// Widening the operands in the contract removes the overflow.
// tag::source[]
#include <cerastes.h>

int larger(int a, int b)
    CERASTES_REQUIRES((long long)a + b > 0)
    CERASTES_ENSURES(r, r >= a && r >= b)
{
  return a > b ? a : b;
}
// end::source[]
