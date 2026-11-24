// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 items 3 and 7 (S45 (a) guard_n): the quantifier as an operand of || (n == 0 ||) and as an arm of ?: (c != 0 ? forall : true): the guard stands outside the quantifier, false guard asks nothing, true guard asks the precondition row
// Pre-registered from record 0061 item 7 (Part B), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

int by_guard(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && (n == 0 || cerastes_forall(nonneg, a, n)))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r >= 0)
{
  if (n == 0) {
    return 0;
  }
  return a[0];
}

int by_choice(const int *a, int n, int c)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n > 0 && (c != 0 ? cerastes_forall(nonneg, a, n) : true))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && (c == 0 || r >= 0))
{
  (void)n;
  if (c != 0) {
    return a[0];
  }
  return 0;
}

int use(void)
    CERASTES_ENSURES(r, r >= 0)
{
  int a[2] = {1, 2};
  int x = by_guard(a, 2);
  int y = by_choice(a, 2, 1);
  (void)y;
  CERASTES_CHECK(y >= 0);
  return x;
}
