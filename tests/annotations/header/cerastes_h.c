// expect: golden
// expect: native-ok
// twin-exempt: C23 header coverage test; the rules it touches have their negatives under the per-attribute positives (c_invariant_on_null and c_requires_on_statement name contracts/c_mode.c)
// Pins: <cerastes.h> in C23: every macro usable in C (the C++ namespace part is
// not seen), the header included twice, and the macros expand the same way.
#include <cerastes.h>
#include <cerastes.h>

CERASTES_PREDICATE bool nonneg(int x)
{
  return x >= 0;
}

CERASTES_PURE int plus1(int x) CERASTES_REQUIRES(x > 0) CERASTES_ENSURES(r, r == x + 1)
{
  return x + 1;
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
