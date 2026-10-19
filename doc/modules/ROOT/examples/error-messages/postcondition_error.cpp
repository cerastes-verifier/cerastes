// expect: error postcondition @ L10
// A program that fails with the postcondition error.
// tag::source[]
#include <cerastes.h>

int double_it(int x)
    CERASTES_REQUIRES(0 <= x && x <= 1000)
    CERASTES_ENSURES(r, r == 2 * x)
{
  return 2 * x + 1;
}
// end::source[]
