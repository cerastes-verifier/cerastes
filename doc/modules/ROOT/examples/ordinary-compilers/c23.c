// expect: golden
// expect: native-ok
// C23: the macros work in C; the specification functions are C++ only.
// tag::c[]
#include <cerastes.h>

int abs_val(int x) CERASTES_REQUIRES(x > -1000) CERASTES_ENSURES(r, r >= 0)
{
  return x < 0 ? -x : x;
}

CERASTES_PREDICATE bool nonneg(int x)
{
  return x >= 0;
}
// end::c[]
