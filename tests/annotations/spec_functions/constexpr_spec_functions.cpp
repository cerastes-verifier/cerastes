// expect: golden
// expect: native-ok
// Pins: R4a: a constexpr predicate and a constexpr pure function used only in contracts are fine (the base of the constant-context negatives)
#include <cerastes.h>

CERASTES_PREDICATE constexpr bool q(int x) { return x >= 0; }
CERASTES_PURE constexpr int p(int x) { return x + 1; }

int bump(int x) CERASTES_REQUIRES(q(x) && x < 100) CERASTES_ENSURES(r, r == p(x))
{
  return x + 1;
}
