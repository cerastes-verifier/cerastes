// expect: error postcondition @ L12
// mode: c23, c++20
// Without the abort() test the postcondition's n % 2 == 0 does not follow.
// Derived from calls_abort.c.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int checked_half(int n)
    CERASTES_ENSURES(r, r == n / 2 && n % 2 == 0)
{
  return n / 2;
}
// end::source[]
