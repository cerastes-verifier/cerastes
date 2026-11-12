// expect: error permission @ L11
// mode: c23, c++20
// twin: tests/memory/range_call_partner_trusted.c
// Pins: 0060 item 3 (audit B1, abl/C1): a bodiless requires(n == 0 || owns_range(p, n)) ensures(owns_range(p, n)) called with (a, 0): the f__wf row reports permission at the ensures atom (it returns p[0..n) on a path where its requires did not take it). Nothing else is expected: the call's ensures then hands back a liveness fraction the caller never gave, so the caller's state is impossible and the check(0) after the call verifies vacuously (as abl/C1's assert false did): the contract's error is the report
// Pre-registered from record 0060 items 3 and 8 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

void g(int *p, int n)
    CERASTES_REQUIRES(n == 0 || cerastes_owns_range(p, n))
    CERASTES_ENSURES(cerastes_owns_range(p, n));

int use(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a[2] = {1, 2};
  g(a, 0);
  CERASTES_CHECK(0);
  return 1;
}
