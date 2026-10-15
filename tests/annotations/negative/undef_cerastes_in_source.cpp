// expect: frontend-error cerastes-cerastes_macro_changed @ L6
// expect: native-ok
// twin: tests/annotations/header/cerastes_h.cpp
// Mutation of the twin: `#undef __CERASTES__` before the include silently drops every annotation (every CERASTES_* macro expands to nothing); the front end defines the macro itself, so changing it is an error.
// Pins: only this diagnostic is reported (the body has no specification-function call that would also fail once the macros are empty).
#undef __CERASTES__
#include <cerastes.h>

CERASTES_PREDICATE bool nonneg(int x)
{
  return x >= 0;
}

int everything(int n) CERASTES_REQUIRES(n >= 0) CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n) while (i < n) {
    i += 1;
  }
  CERASTES_CHECK(i == n);
  CERASTES_FOLD(nonneg(i));
  CERASTES_UNFOLD(nonneg(i));
  return i;
}
