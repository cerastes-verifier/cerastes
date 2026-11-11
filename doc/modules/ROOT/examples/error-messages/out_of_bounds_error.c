// expect: error out-of-bounds @ L11
// mode: c23, c++20
// tag::source[]
#include <cerastes.h>

int pick(int i)
    CERASTES_REQUIRES(0 <= i && i <= 3)
{
  int prices[3] = {10, 20, 30};
  int k = i;
  return prices[k];
}
// end::source[]
