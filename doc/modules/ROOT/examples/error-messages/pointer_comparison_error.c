// expect: error pointer-comparison @ L11
// mode: c23, c++20
// tag::source[]
#include <cerastes.h>

bool same(const int *p, const int *q)
    CERASTES_REQUIRES(cerastes_live(p) && cerastes_live(q))
    CERASTES_ENSURES(cerastes_live(p) && cerastes_live(q))
{
  // p or q may point one past the end of an object
  return p == q;
}
// end::source[]
