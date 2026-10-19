// expect: golden
// expect: native-ok
// Pins: R4b: a predicate called only in contracts (the base of the initializer and pointer-to-member negatives)
#include <cerastes.h>

CERASTES_PREDICATE bool nonneg(int x)
{
  return x >= 0;
}

int ident(int x) CERASTES_REQUIRES(nonneg(x)) CERASTES_ENSURES(r, r == x)
{
  return x;
}
