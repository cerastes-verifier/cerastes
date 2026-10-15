// expect: golden
// expect: native-ok
// Pins: fold and unfold on null statements; the argument is a call to a
// predicate function with parameters/locals as arguments (a routine node for
// the callee, not an evaluated call).
#include <cerastes.h>

CERASTES_PREDICATE bool in_range(int lo, int hi, int v)
{
  return lo <= v && v < hi;
}

int use([[maybe_unused]] int lo, [[maybe_unused]] int hi, int v) CERASTES_REQUIRES(in_range(lo, hi, v))
{
  CERASTES_UNFOLD(in_range(lo, hi, v));
  int w = v + 0;
  CERASTES_FOLD(in_range(lo, hi, w));
  return w;
}
