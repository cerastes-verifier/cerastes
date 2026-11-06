// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// Rejected: a contract with no body promises a place on a parameter that its requires did not take; the caller may already hold it.
// tag::source[]
#include <cerastes.h>

void give(int *p)
    CERASTES_REQUIRES(cerastes_live(p))
    CERASTES_ENSURES(cerastes_owns(*p));

void caller(int *q)
    CERASTES_REQUIRES(cerastes_live(q))
{
  give(q);
}
// end::source[]
