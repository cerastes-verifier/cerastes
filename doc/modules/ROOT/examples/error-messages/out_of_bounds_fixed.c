// expect: verified
// expect: native-ok
// mode: c23, c++20
// The same program, fixed: the precondition keeps the index inside the array.
// tag::source[]
#include <cerastes.h>

int pick(int i)
    CERASTES_REQUIRES(0 <= i && i < 3)
    CERASTES_ENSURES(r, r == 10 * (i + 1))
{
  int prices[3] = {10, 20, 30};
  int k = i;
  return prices[k];
}
// end::source[]
