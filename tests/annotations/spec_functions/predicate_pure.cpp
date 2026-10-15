// expect: golden
// expect: native-ok
// Pins: predicate and pure as prefix macros (they expand to the attribute plus
// `inline`): a predicate with a parameter, a pure function with two
// parameters, and a pure function that also carries a contract.
#include <cerastes.h>

CERASTES_PREDICATE bool nonneg(int x)
{
  return x >= 0;
}

CERASTES_PURE int clamp_hi(int x, int hi)
{
  return x < hi ? x : hi;
}

CERASTES_PURE int dbl(int x) CERASTES_REQUIRES(x >= 0) CERASTES_ENSURES(r, r >= x)
{
  return x + x;
}
