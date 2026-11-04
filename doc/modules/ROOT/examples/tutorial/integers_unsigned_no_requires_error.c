// expect: error overflow @ L9
// expect: error overflow @ L11
// mode: c23, c++20
// The precondition is gone: a - b goes below zero, in the contract (L9) and in the body (L11).
// tag::source[]
#include <cerastes.h>

unsigned difference(unsigned a, unsigned b)
    CERASTES_ENSURES(r, r == a - b)
{
  return a - b;
}
// end::source[]
