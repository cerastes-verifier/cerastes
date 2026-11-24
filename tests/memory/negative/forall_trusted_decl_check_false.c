// expect: error check @ L23
// mode: c23, c++20
// twin: tests/memory/forall_trusted_decl.c
// Pins: 0061 item 7 (vacuity twin, audit round 2 issue 3): the trusted quantified contract is consistent (a definition satisfying it exists: one that fills every element non-negatively), so a check(false) after the call fails and the call does not make the state vacuous; an INCONSISTENT trusted contract (one no definition satisfies, for example reads_range plus an ensures that changes the elements) would make the caller's state vacuous, as for scalars, inside the single-unit assumption (trusted contracts are assumed, record 0012 item 1)
// Pre-registered from record 0061 item 7 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

void fill_nonneg(int *a, int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n))
    CERASTES_ENSURES(cerastes_owns_range(a, n) && cerastes_forall(nonneg, a, n));

int use(void)
    CERASTES_ENSURES(r, r >= 0)
{
  int a[3] = {0, 0, 0};
  fill_nonneg(a, 3);
  CERASTES_CHECK(0 == 1);
  return a[1];
}
