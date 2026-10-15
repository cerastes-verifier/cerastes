// expect: golden
// expect: native-ok
// predicate and pure functions, fold and unfold.
#include <cerastes.h>

// tag::predicate[]
CERASTES_PREDICATE bool in_range(int lo, int hi, int v)
{
  return lo <= v && v < hi;
}
// end::predicate[]

// tag::pure[]
CERASTES_PURE int clamp_hi(int x, int hi)
{
  return x < hi ? x : hi;
}
// end::pure[]

// tag::fold[]
int use([[maybe_unused]] int lo, [[maybe_unused]] int hi, int v) CERASTES_REQUIRES(in_range(lo, hi, v))
{
  CERASTES_UNFOLD(in_range(lo, hi, v));
  int w = v + 0;
  CERASTES_FOLD(in_range(lo, hi, w));
  return w;
}
// end::fold[]
